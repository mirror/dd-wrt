/* BGP-4, BGP-4+ daemon program
 * Copyright (C) 1996, 97, 98, 99, 2000 Kunihiro Ishiguro
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

#include "prefix.h"
#include "thread.h"
#include "buffer.h"
#include "stream.h"
#include "table.h"
#include "command.h"
#include "sockunion.h"
#include "network.h"
#include "memory.h"
#include "filter.h"
#include "routemap.h"
#include "str.h"
#include "log.h"
#include "plist.h"
#include "linklist.h"

#include "bgpd/bgpd.h"
#include "bgpd/bgp_aspath.h"
#include "bgpd/bgp_route.h"
#include "bgpd/bgp_dump.h"
#include "bgpd/bgp_debug.h"
#include "bgpd/bgp_community.h"
#include "bgpd/bgp_attr.h"
#include "bgpd/bgp_clist.h"
#include "bgpd/bgp_fsm.h"
#include "bgpd/bgp_packet.h"
#include "bgpd/bgp_zebra.h"
#include "bgpd/bgp_open.h"
#include "bgpd/bgp_filter.h"
#include "bgpd/bgp_nexthop.h"
#include "bgpd/bgp_damp.h"
#include "bgpd/bgp_mplsvpn.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

/* All BGP instance. */
struct list *bgp_list;

/* All peer instance. */
struct list *peer_list;

/* BGP multiple instance flag. */
int bgp_multiple_instance;

/* Enable BGP mutliple instance configuration. */
DEFUN (bgp_multiple_instance_func,
       bgp_multiple_instance_cmd,
       "bgp multiple-instance",
       "BGP specific commands\n"
       "Enable bgp multiple instance\n")
{
  bgp_multiple_instance = 1;
  return CMD_SUCCESS;
}

/* Disable BGP multiple instance. */
DEFUN (no_bgp_multiple_instance,
       no_bgp_multiple_instance_cmd,
       "no bgp multiple-instance",
       NO_STR
       "BGP specific commands\n"
       "BGP multiple instance\n")
{
  if (bgp_list->count > 1)
    {
      vty_out (vty, "There are more than two BGP instances%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }
  bgp_multiple_instance = 0;
  return CMD_SUCCESS;
}

/* Peer group cofiguration. */
struct peer_group *
peer_group_new ()
{
  struct peer_group *group;

  group = XMALLOC (MTYPE_PEER_GROUP, sizeof (struct peer_group));
  memset (group, 0, sizeof (struct peer_group));
  return group;
}

void
peer_group_free (struct peer_group *group)
{
  XFREE (MTYPE_PEER_GROUP, group);
}

struct peer_group *
peer_group_lookup (struct list *glist, char *name)
{
  struct peer_group *group;
  struct listnode *nn;

  LIST_LOOP (glist, group, nn)
    {
      if (strcmp(group->name, name) == 0)
	return group;
    }
  return NULL;
}

int
peer_group_get (struct vty *vty, char *name, int afi, int safi)
{
  struct bgp *bgp;
  struct peer_group *group;

  bgp = vty->index;
  group = peer_group_lookup (bgp->peer_group, name);

  if (group)
    {
      vty_out (vty, "Same name peer-group already exists%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  group = peer_group_new ();
  group->name = strdup (name);
  group->peer_conf = list_new ();
  listnode_add (bgp->peer_group, group);

  return CMD_SUCCESS;
}

int
peer_conf_peer_group (struct vty *vty, char *peer_str, char *group_str)
{
  struct bgp *bgp;
  struct peer_group *group;

  bgp = vty->index;

  group = peer_group_lookup (bgp->peer_group, group_str);

  return CMD_SUCCESS;
}

DEFUN (neighbor_peer_group,
       neighbor_peer_group_cmd,
       "neighbor WORD peer-group",
       NEIGHBOR_STR
       "Neighbor tag\n"
       "Configure peer-group\n")
{
  return peer_group_get (vty, argv[0], 0, 0);
}

DEFUN (neighbor_peer_group_remote_as,
       neighbor_peer_group_remote_as_cmd,
       "neighbor WORD remote-as <1-65535>",
       NEIGHBOR_STR
       "Neighbor tag\n"
       "Specify a BGP neighbor\n"
       "AS of remote neighbor\n")
{
  struct bgp *bgp;
  struct peer_group *group;
  char *endptr = NULL;
  as_t as;

  bgp = vty->index;
  group = peer_group_lookup (bgp->peer_group, argv[0]);

  if (!group)
    {
      vty_out (vty, "Please configure peer-group first%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  if (group->peer_conf->count)
    {
      vty_out (vty, "Can't configure AS number for existance peer%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Convert string to number. */
  as = strtoul (argv[1], &endptr, 10);
  if (as == ULONG_MAX || *endptr != '\0' || as < 1 || as > 65535)
    {
      vty_out (vty, "AS value error%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  group->as = as;

  return CMD_SUCCESS;
}

DEFUN (neighbor_set_peer_group,
       neighbor_set_peer_group_cmd,
       NEIGHBOR_CMD "peer-group WORD",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Member of the peer-group"
       "peer-group name\n")
{
  return peer_conf_peer_group (vty, argv[0], argv[1]);
}

/* Set BGP's router identifier. */
int
bgp_router_id_set (struct vty *vty, char *id_str)
{
  struct bgp *bgp;
  struct in_addr id;
  int ret;
  struct peer_conf *conf;
  struct listnode *nn;

  ret = inet_aton (id_str, &id);
  if (!ret)
    {
      vty_out (vty, "Malformed bgp router identifier%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Set identifier to BGP structure. */
  bgp = vty->index;

  if (CHECK_FLAG (bgp->config, BGP_CONFIG_ROUTER_ID)
      && bgp->id.s_addr == id.s_addr)
    return CMD_SUCCESS;

  bgp->id = id;
  SET_FLAG (bgp->config, BGP_CONFIG_ROUTER_ID);

  /* Set all peer's local identifier with this value. */
  LIST_LOOP (bgp->peer_conf, conf, nn)
    {
      conf->peer->local_id = id;
      /* Reset all BGP sessions */
      BGP_EVENT_ADD (conf->peer, BGP_Stop);
    }

  return CMD_SUCCESS;
}

/* Unset BGP router identifier. */
int
bgp_router_id_unset (struct vty *vty, char *id_str)
{
  int ret;
  struct bgp *bgp;
  struct in_addr id;
  struct peer_conf *conf;
  struct listnode *nn;

  bgp = vty->index;
  
  if (id_str)
    {
      ret = inet_aton (id_str, &id);
      if (!ret)
	{
	  vty_out (vty, "Malformed bgp router identifier%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}

      if (!IPV4_ADDR_SAME (&bgp->id, &id))
	{
	  vty_out (vty, "bgp router-id doesn't match%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}
    }

  if (! CHECK_FLAG (bgp->config, BGP_CONFIG_ROUTER_ID))
    return CMD_SUCCESS;

  bgp->id.s_addr = 0;
  UNSET_FLAG (bgp->config, BGP_CONFIG_ROUTER_ID);

  LIST_LOOP (bgp->peer_conf, conf, nn)
    {
      conf->peer->local_id.s_addr = 0;
    }

  /* Set router-id from interface's address. */
  bgp_if_update_all ();

  /* Reset all BGP sessions */
  LIST_LOOP (bgp->peer_conf, conf, nn)
    {
      BGP_EVENT_ADD (conf->peer, BGP_Stop);
    }

  return CMD_SUCCESS;
}

DEFUN (bgp_router_id, bgp_router_id_cmd,
       "bgp router-id A.B.C.D",
       "BGP specific commands\n"
       "Override configured router identifier\n"
       "Manually configured router identifier\n")
{
  return bgp_router_id_set (vty, argv[0]);
}

DEFUN (no_bgp_router_id, no_bgp_router_id_cmd,
       "no bgp router-id",
       NO_STR
       "BGP specific commands\n"
       "Override configured router identifier\n")
{
  if (argc == 0)
    return bgp_router_id_unset (vty, NULL);
  
  return bgp_router_id_unset (vty, argv[0]);
}

ALIAS (no_bgp_router_id, no_bgp_router_id_val_cmd,
       "no bgp router-id A.B.C.D",
       NO_STR
       "BGP specific commands\n"
       "Override configured router identifier\n"
       "Manually configured router identifier\n")

/* Set BGP's global timers. */
int
bgp_timers_set (struct vty *vty, char *keep_str, char *hold_str, int set)
{
  struct bgp *bgp;
  struct peer_conf *conf;
  struct listnode *nn;
  unsigned long keepalive = 0;
  unsigned long holdtime = 0;
  char *endptr = NULL;

  if (set)
    {
      /* keepalive value check. */
      keepalive = strtoul (keep_str, &endptr, 10);

      if (keepalive == ULONG_MAX || *endptr != '\0')
	{
	  vty_out (vty, "%% keepalive time value must be positive integer%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}
      if (keepalive > 65535)
	{
	  vty_out (vty, "%% keepalive time value must be <0-65535>%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}

      /* Holdtime value check. */
      holdtime = strtoul (hold_str, &endptr, 10);

      if (holdtime == ULONG_MAX || *endptr != '\0')
	{
	  vty_out (vty, "%% hold time value must be positive integer%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}
      if (holdtime > 65535)
	{
	  vty_out (vty, "%% hold time value must be <0,3-65535>%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}
      if (holdtime < 3 && holdtime != 0)
	{
	  vty_out (vty, "%% hold time value must be either 0 or greater than 3%s",
	       VTY_NEWLINE);
	  return CMD_WARNING;
	}
    }

  /* Set identifier to BGP structure. */
  bgp = vty->index;

  /* Set value to the configuration. */
  if (set)
    {
      bgp->default_holdtime = holdtime;
      bgp->default_keepalive = (keepalive < holdtime / 3 ? keepalive : holdtime / 3);
    }
  else
    {
      bgp->default_holdtime = BGP_DEFAULT_HOLDTIME;
      bgp->default_keepalive = BGP_DEFAULT_KEEPALIVE;
    }

  /* Set all peer's global timers with this value. */
  LIST_LOOP (bgp->peer_conf, conf, nn)
    {
      conf->peer->global_holdtime = bgp->default_holdtime;
      conf->peer->global_keepalive = bgp->default_keepalive;
    }

  return CMD_SUCCESS;
}

DEFUN (bgp_timers, bgp_timers_cmd,
       "timers bgp <0-65535> <0-65535>",
       "Adjust routing timers\n"
       "BGP timers\n"
       "Keepalive interval\n"
       "Holdtime\n")
{
  return bgp_timers_set (vty, argv[0], argv[1], 1);
}

DEFUN (no_bgp_timers, no_bgp_timers_cmd,
       "no timers bgp",
       NO_STR
       "Adjust routing timers\n"
       "BGP timers\n")
{
  return bgp_timers_set (vty, NULL, NULL, 0);
}

/* BGP's cluster-id control. */
int
bgp_cluster_id_set (struct vty *vty, char *cluster_str)
{
  int ret;
  struct bgp *bgp;
  struct in_addr cluster;

  ret = inet_aton (cluster_str, &cluster);
  if (!ret)
    {
      vty_out (vty, "Malformed bgp cluster identifier%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  bgp = vty->index;
  bgp->cluster = cluster;
  bgp->config |= BGP_CONFIG_CLUSTER_ID;

  return CMD_SUCCESS;
}

int
bgp_cluster_id_unset (struct vty *vty, char *cluster_str)
{
  int ret;
  struct bgp *bgp;
  struct in_addr cluster;

  bgp = vty->index;

  if (cluster_str)
    {
      ret = inet_aton (cluster_str, &cluster);
      if (!ret)
	{
	  vty_out (vty, "Malformed bgp cluster identifier%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}
      if (! IPV4_ADDR_SAME (&bgp->cluster, &cluster))
	{
	  vty_out (vty, "bgp cluster-id doesn't match%s", VTY_NEWLINE);
	  return CMD_WARNING;
      }
    }
  bgp->cluster.s_addr = 0;
  bgp->config &= ~BGP_CONFIG_CLUSTER_ID;

  return CMD_SUCCESS;
}

DEFUN (bgp_cluster_id, bgp_cluster_id_cmd,
       "bgp cluster-id A.B.C.D",
       "BGP specific commands\n"
       "Configure Route-Reflector Cluster-id\n"
       "Route-Reflector Cluster-id in IP address format\n")
{
  return bgp_cluster_id_set (vty, argv[0]);
}

ALIAS (bgp_cluster_id, bgp_cluster_id32_cmd,
       "bgp cluster-id <1-4294967295>",
       "BGP specific commands\n"
       "Configure Route-Reflector Cluster-id\n"
       "Route-Reflector Cluster-id as 32 bit quantity\n")

DEFUN (no_bgp_cluster_id, no_bgp_cluster_id_cmd,
       "no bgp cluster-id",
       NO_STR
       "BGP specific commands\n"
       "Configure Route-Reflector Cluster-id\n")
{
  if (argc == 0)
    return bgp_cluster_id_unset (vty, NULL);
  
  return bgp_cluster_id_unset (vty, argv[0]);
}

ALIAS (no_bgp_cluster_id, no_bgp_cluster_id_val_cmd,
       "no bgp cluster-id A.B.C.D",
       NO_STR
       "BGP specific commands\n"
       "Configure Route-Reflector Cluster-id\n"
       "Route-Reflector Cluster-id in IP address format\n")

int
bgp_confederation_id_set (struct vty *vty, char *id_str)
{
  struct bgp *bgp;
  as_t as = 0;
  char *endptr = NULL;
  struct peer *peer;
  struct listnode *nn;
  int old_confed_flag;  /* Old Confederations status */

  bgp = vty->index;

  if (id_str)
    {
      as = strtoul (id_str, &endptr, 10);
      if (as == ULONG_MAX || *endptr != '\0' || as < 1 || as > 65535)
	{
	  vty_out (vty, "AS value error%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}

      /* Remember - were we doing CONFEDs before? */
      old_confed_flag = CHECK_FLAG (bgp->config, BGP_CONFIG_CONFEDERATION);
      bgp->confederation_id = as;
      SET_FLAG (bgp->config, BGP_CONFIG_CONFEDERATION);

      /*
       * how to handle already setup peers?
       * Answer - If we were doing CONFEDs already 
       *               - this is just an external AS change
       *               - just Reset EBGP sessions, not CONFED sessions
       *          If we were not doing CONFEDs before
       *               - Reset all EBGP sessions
       */
      LIST_LOOP (peer_list, peer, nn)
	{
	  /* We're looking for peers who's AS is not local or part of
             our CONFED*/
	  if(old_confed_flag)
	    {
	      if (peer_sort (peer) == BGP_PEER_EBGP)
		{
		  peer->local_as = as;
		  BGP_EVENT_ADD (peer, BGP_Stop);
		}
	    }
	  else
	    {
	      /* Not doign CONFEDs before, so reset every non-local
                 session */
	      if (peer_sort (peer) != BGP_PEER_IBGP)
		{
		  /* Reset the local_as to be our EBGP one */
		  if (peer_sort (peer) == BGP_PEER_EBGP)
		    peer->local_as = as;
		  BGP_EVENT_ADD (peer, BGP_Stop);
		}
	    }
	}
      return CMD_SUCCESS;
    }
  else
    {
      vty_out (vty, "No AS Number provided%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  return CMD_WARNING;
}

int
bgp_confederation_id_unset (struct vty *vty, char *id_str)
{
  struct bgp *bgp;
  as_t as;
  char *endptr = NULL;
  struct peer *peer;
  struct listnode *nn;

  bgp = vty->index;

  if (id_str)
    {
      as = strtoul (id_str, &endptr, 10);
      if (as == ULONG_MAX || *endptr != '\0' || as < 1 || as > 65535)
	{
	  vty_out (vty, "%% AS value error%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}
      
      if (bgp->confederation_id != as)
	{
	  vty_out (vty, "%% AS value does not match%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}
      
      bgp->confederation_id = 0;
      UNSET_FLAG (bgp->config, BGP_CONFIG_CONFEDERATION);
      
      /*
       * How do we handle all EBGP peers if we have no external AS?
       * Assumption - No Confed ID == no CONFEDERATIONS, so
       * clear all EBGP *AND* CONFED peers and bring up with no spoofing.
       */
      LIST_LOOP (peer_list, peer, nn)
	{
	  /* We're looking for peers who's AS is not local */
	  if (peer_sort (peer) != BGP_PEER_IBGP)
	    {
	      peer->local_as = bgp->as;
	      BGP_EVENT_ADD (peer, BGP_Stop);
	    }
	}   
      return CMD_SUCCESS;
    }
  else
    {
      vty_out (vty, "%% No AS Number provided%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  return CMD_WARNING;     
}

/* Is an AS part of the confed or not? */
int
bgp_confederation_peers_check (struct bgp *bgp, as_t as)
{
  int i;

  if (bgp == NULL)
    return 0;

  for(i = 0; i < bgp->confederation_peers_cnt; i++)
    {
      if (bgp->confederation_peers[i] == as)
	return 1;
    }

  return 0;
}

/* Add an AS to the CONFED set */
void
bgp_confederation_peers_add (struct bgp *bgp, as_t as)
{
  bgp->confederation_peers = XREALLOC (MTYPE_BGP_CONFED_LIST, 
				       bgp->confederation_peers,
				       bgp->confederation_peers_cnt + 1);
  bgp->confederation_peers[bgp->confederation_peers_cnt] = as;
  bgp->confederation_peers_cnt++;
}

void
bgp_confederation_peers_remove (struct bgp *bgp, as_t as)
{
  int i;
  int j;

  for(i = 0; i < bgp->confederation_peers_cnt; i++)
    {
      if(bgp->confederation_peers[i] == as)
	{
	  /* Remove this entry */
	  for(j = i+1; j < bgp->confederation_peers_cnt; j++)
	    {
	      bgp->confederation_peers[j-1] = bgp->confederation_peers[j];
	    }
	}
    }

  bgp->confederation_peers_cnt--;

  if (bgp->confederation_peers_cnt == 0)
    {
      bgp->confederation_peers = NULL;
    }
  else
    {
      bgp->confederation_peers = XREALLOC(MTYPE_BGP_CONFED_LIST,
					  bgp->confederation_peers,
					  bgp->confederation_peers_cnt);
    }
}

int
bgp_confederation_peers_set (struct vty *vty, int argc, char *argv[])
{
  struct bgp *bgp;
  as_t as;
  int i;
  char *endptr = NULL;

  bgp = vty->index;

  for(i = 0; i < argc; i++)
    {
      as = strtoul (argv[i], &endptr, 10);
      if (as == ULONG_MAX || as < 1 || as > 65535)
	{
	  vty_out (vty, "AS Value error (%s), ignoring%s",
		   argv[i], VTY_NEWLINE);
	}
      else
	{
	  if (bgp->as == as)
	    {
	      vty_out (vty, "%% Local member-AS not allowed in confed peer list%s",
			VTY_NEWLINE);
	      continue;
	    }
	  if (! bgp_confederation_peers_check (bgp, as))
	    {
	      struct peer *peer;
	      struct listnode *nn;

	      /* Its not there already, so add it */
	      bgp_confederation_peers_add (bgp, as);

	      /* Now reset any peer who's remote AS has just joined
		 the CONFED unless its an iBGP peer */
	      LIST_LOOP (peer_list, peer, nn)
		{
		  if (peer->as == as)
		    {
		      /* If the AS added to the list */
		      if (CHECK_FLAG (bgp->config, BGP_CONFIG_CONFEDERATION))
			{
			  peer->local_as = bgp->as;
			  BGP_EVENT_ADD (peer, BGP_Stop);
			}
		    }
		}
	    }
	  else
	    {
	      /* Silently ignore repeated ASs */
	    }
	}
    }
  return CMD_SUCCESS;
}

int
bgp_confederation_peers_unset (struct vty *vty, int argc, char *argv[])
{
  struct bgp *bgp;
  as_t as;
  int i;
  char *endptr = NULL;

  bgp = vty->index;

  for(i = 0; i < argc; i++)
    {
      as = strtoul (argv[i], &endptr, 10);

      if (as == ULONG_MAX || as < 1 || as > 65535)
	continue;

      if (! bgp_confederation_peers_check(bgp, as))
	{
	  /* Its not there already, so silently ignore this*/
	}
      else
	{
	  struct peer *peer;
	  struct listnode *nn;
	  
	  /* Its there - we need to remove it */
	  bgp_confederation_peers_remove (bgp, as);

	  /* Now reset any peer who's remote AS has just been
	     removed from the CONFED */
	  LIST_LOOP (peer_list, peer, nn)
	    {
	      if (peer->as == as)
		{
		  /* Set the peer's local-as correctly */
		  if (CHECK_FLAG (bgp->config, BGP_CONFIG_CONFEDERATION))
		    {
		      peer->local_as = bgp->confederation_id;
		      BGP_EVENT_ADD (peer, BGP_Stop);
		    }
		}
	    }
	}
    }
  return CMD_SUCCESS;
}

void
bgp_confederation_peers_print (struct vty *vty, struct bgp *bgp)
{
  int i;

  for(i = 0; i < bgp->confederation_peers_cnt; i++)
    {
      vty_out(vty, " ");

      vty_out(vty, "%d", bgp->confederation_peers[i]);
    }
}

DEFUN (bgp_confederation_peers, bgp_confederation_peers_cmd,
       "bgp confederation peers .<1-65535>",
       "BGP specific commands\n"
       "AS confederation parameters\n"
       "Peer ASs in BGP confederation\n"
       AS_STR)
{
  return bgp_confederation_peers_set(vty, argc, argv);
}

DEFUN (bgp_confederation_identifier, bgp_confederation_identifier_cmd,
       "bgp confederation identifier <1-65535>",
       "BGP specific commands\n"
       "AS confederation parameters\n"
       "AS number\n"
       "Set routing domain confederation AS\n")
{
  return bgp_confederation_id_set(vty, argv[0]);
}

DEFUN (no_bgp_confederation_peers, no_bgp_confederation_peers_cmd,
       "no bgp confederation peers .<1-65535>",
       NO_STR
       "BGP specific commands\n"
       "AS confederation parameters\n"
       "Peer ASs in BGP confederation\n"
       AS_STR)
{
  return bgp_confederation_peers_unset(vty, argc, argv);
}

DEFUN (no_bgp_confederation_identifier, no_bgp_confederation_identifier_cmd,
       "no bgp confederation identifier <1-65535>",
       NO_STR
       "BGP specific commands\n"
       "AS confederation parameters\n"
       "AS number\n"
       "Set routing domain confederation AS\n")
{
  return bgp_confederation_id_unset(vty, argv[0]);
}

/* "no bgp client-to-client reflection" configuration. */
DEFUN (no_bgp_client_to_client_reflection,
       no_bgp_client_to_client_reflection_cmd,
       "no bgp client-to-client reflection",
       NO_STR
       "BGP specific commands\n"
       "Configure client to client route reflection\n"
       "reflection of routes allowed\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  SET_FLAG (bgp->config, BGP_CONFIG_NO_CLIENT_TO_CLIENT);
  return CMD_SUCCESS;
}

DEFUN (bgp_client_to_client_reflection,
       bgp_client_to_client_reflection_cmd,
       "bgp client-to-client reflection",
       "BGP specific commands\n"
       "Configure client to client route reflection\n"
       "reflection of routes allowed\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  UNSET_FLAG (bgp->config, BGP_CONFIG_NO_CLIENT_TO_CLIENT);
  return CMD_SUCCESS;
}

/* "bgp always-compare-med" configuration. */
DEFUN (bgp_always_compare_med,
       bgp_always_compare_med_cmd,
       "bgp always-compare-med",
       "BGP specific commands\n"
       "Allow comparing MED from different neighbors\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  SET_FLAG (bgp->config, BGP_CONFIG_ALWAYS_COMPARE_MED);
  return CMD_SUCCESS;
}

DEFUN (no_bgp_always_compare_med,
       no_bgp_always_compare_med_cmd,
       "no bgp always-compare-med",
       NO_STR
       "BGP specific commands\n"
       "Allow comparing MED from different neighbors\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  UNSET_FLAG (bgp->config, BGP_CONFIG_ALWAYS_COMPARE_MED);
  return CMD_SUCCESS;
}

/* "bgp deterministic-med" configuration. */
DEFUN (bgp_deterministic_med,
       bgp_deterministic_med_cmd,
       "bgp deterministic-med",
       "BGP specific commands\n"
       "Pick the best-MED path among paths advertised from the neighboring AS\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  SET_FLAG (bgp->config, BGP_CONFIG_DETERMINISTIC_MED);
  return CMD_SUCCESS;
}

DEFUN (no_bgp_deterministic_med,
       no_bgp_deterministic_med_cmd,
       "no bgp deterministic-med",
       NO_STR
       "BGP specific commands\n"
       "Pick the best-MED path among paths advertised from the neighboring AS\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  UNSET_FLAG (bgp->config, BGP_CONFIG_DETERMINISTIC_MED);
  return CMD_SUCCESS;
}

/* "bgp enforce-first-as" configuration. */
DEFUN (bgp_enforce_first_as,
       bgp_enforce_first_as_cmd,
       "bgp enforce-first-as",
       BGP_STR
       "Enforce the first AS for EBGP routes\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  SET_FLAG (bgp->config, BGP_CONFIG_ENFORCE_FIRST_AS);
  return CMD_SUCCESS;
}

DEFUN (no_bgp_enforce_first_as,
       no_bgp_enforce_first_as_cmd,
       "no bgp enforce-first-as",
       NO_STR
       BGP_STR
       "Enforce the first AS for EBGP routes\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  UNSET_FLAG (bgp->config, BGP_CONFIG_ENFORCE_FIRST_AS);
  return CMD_SUCCESS;
}

/* "bgp bestpath compare-routerid" configuration.  */
DEFUN (bgp_bestpath_compare_router_id,
       bgp_bestpath_compare_router_id_cmd,
       "bgp bestpath compare-routerid",
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "Compare router-id for identical EBGP paths\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  SET_FLAG (bgp->config, BGP_CONFIG_COMPARE_ROUTER_ID);
  return CMD_SUCCESS;
}

DEFUN (no_bgp_bestpath_compare_router_id,
       no_bgp_bestpath_compare_router_id_cmd,
       "no bgp bestpath compare-routerid",
       NO_STR
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "Compare router-id for identical EBGP paths\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  UNSET_FLAG (bgp->config, BGP_CONFIG_COMPARE_ROUTER_ID);
  return CMD_SUCCESS;
}

/* "bgp bestpath as-path ignore" configuration.  */
DEFUN (bgp_bestpath_aspath_ignore,
       bgp_bestpath_aspath_ignore_cmd,
       "bgp bestpath as-path ignore",
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "AS-path attribute\n"
       "Ignore as-path length in selecting a route\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  SET_FLAG (bgp->config, BGP_CONFIG_ASPATH_IGNORE);
  return CMD_SUCCESS;
}

DEFUN (no_bgp_bestpath_aspath_ignore,
       no_bgp_bestpath_aspath_ignore_cmd,
       "no bgp bestpath as-path ignore",
       NO_STR
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "AS-path attribute\n"
       "Ignore as-path length in selecting a route\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  UNSET_FLAG (bgp->config, BGP_CONFIG_ASPATH_IGNORE);
  return CMD_SUCCESS;
}


/* "bgp bestpath med" configuration. */
DEFUN (bgp_bestpath_med,
       bgp_bestpath_med_cmd,
       "bgp bestpath med (confed|missing-as-worst)",
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "MED attribute\n"
       "Compare MED among confederation paths\n"
       "Treat missing MED as the least preferred one\n")
{
  struct bgp *bgp;
  
  bgp = vty->index;

  if (strcmp (argv[0], "confed") == 0)
    SET_FLAG (bgp->config, BGP_CONFIG_MED_CONFED);
  else
    SET_FLAG (bgp->config, BGP_CONFIG_MED_MISSING_AS_WORST);

  return CMD_SUCCESS;
}

DEFUN (bgp_bestpath_med2,
       bgp_bestpath_med2_cmd,
       "bgp bestpath med confed missing-as-worst",
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "MED attribute\n"
       "Compare MED among confederation paths\n"
       "Treat missing MED as the least preferred one\n")
{
  struct bgp *bgp;
  
  bgp = vty->index;
  SET_FLAG (bgp->config, BGP_CONFIG_MED_CONFED);
  SET_FLAG (bgp->config, BGP_CONFIG_MED_MISSING_AS_WORST);
  return CMD_SUCCESS;
}

ALIAS (bgp_bestpath_med2,
       bgp_bestpath_med3_cmd,
       "bgp bestpath med missing-as-worst confed",
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "MED attribute\n"
       "Treat missing MED as the least preferred one\n"
       "Compare MED among confederation paths\n")

DEFUN (no_bgp_bestpath_med,
       no_bgp_bestpath_med_cmd,
       "no bgp bestpath med (confed|missing-as-worst)",
       NO_STR
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "MED attribute\n"
       "Compare MED among confederation paths\n"
       "Treat missing MED as the least preferred one\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  
  if (strcmp (argv[0], "confed") == 0)
    UNSET_FLAG (bgp->config, BGP_CONFIG_MED_CONFED);
  else
    UNSET_FLAG (bgp->config, BGP_CONFIG_MED_MISSING_AS_WORST);

  return CMD_SUCCESS;
}

DEFUN (no_bgp_bestpath_med2,
       no_bgp_bestpath_med2_cmd,
       "no bgp bestpath med confed missing-as-worst",
       NO_STR
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "MED attribute\n"
       "Compare MED among confederation paths\n"
       "Treat missing MED as the least preferred one\n")
{
  struct bgp *bgp;
  
  bgp = vty->index;
  UNSET_FLAG (bgp->config, BGP_CONFIG_MED_CONFED);
  UNSET_FLAG (bgp->config, BGP_CONFIG_MED_MISSING_AS_WORST);
  return CMD_SUCCESS;
}

ALIAS (no_bgp_bestpath_med2,
       no_bgp_bestpath_med3_cmd,
       "no bgp bestpath med missing-as-worst confed",
       NO_STR
       "BGP specific commands\n"
       "Change the default bestpath selection\n"
       "MED attribute\n"
       "Treat missing MED as the least preferred one\n"
       "Compare MED among confederation paths\n")

DEFUN (no_bgp_default_ipv4_unicast,
       no_bgp_default_ipv4_unicast_cmd,
       "no bgp default ipv4-unicast",
       NO_STR
       "BGP specific commands\n"
       "Configure BGP defaults\n"
       "Activate ipv4-unicast for a peer by default\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  SET_FLAG (bgp->config, BGP_CONFIG_NO_DEFAULT_IPV4);
  return CMD_SUCCESS;
}

DEFUN (bgp_default_ipv4_unicast,
       bgp_default_ipv4_unicast_cmd,
       "bgp default ipv4-unicast",
       "BGP specific commands\n"
       "Configure BGP defaults\n"
       "Activate ipv4-unicast for a peer by default\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  UNSET_FLAG (bgp->config, BGP_CONFIG_NO_DEFAULT_IPV4);
  return CMD_SUCCESS;
}

/* "bgp import-check" configuration.  */
DEFUN (bgp_network_import_check,
       bgp_network_import_check_cmd,
       "bgp network import-check",
       "BGP specific commands\n"
       "BGP network command\n"
       "Check BGP network route exists in IGP\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  SET_FLAG (bgp->config, BGP_CONFIG_IMPORT_CHECK);
  return CMD_SUCCESS;
}

DEFUN (no_bgp_network_import_check,
       no_bgp_network_import_check_cmd,
       "no bgp network import-check",
       NO_STR
       "BGP specific commands\n"
       "BGP network command\n"
       "Check BGP network route exists in IGP\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  UNSET_FLAG (bgp->config, BGP_CONFIG_IMPORT_CHECK);
  return CMD_SUCCESS;
}

DEFUN (bgp_default_local_preference,
       bgp_default_local_preference_cmd,
       "bgp default local-preference <0-4294967295>",
       "BGP specific commands\n"
       "Configure BGP defaults\n"
       "local preference (higher=more preferred)\n"
       "Configure default local preference value\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  bgp->default_local_pref = atoi(argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_bgp_default_local_preference,
       no_bgp_default_local_preference_cmd,
       "no bgp default local-preference",
       NO_STR
       "BGP specific commands\n"
       "Configure BGP defaults\n"
       "local preference (higher=more preferred)\n")
{
  struct bgp *bgp;

  bgp = vty->index;
  bgp->default_local_pref = BGP_DEFAULT_LOCAL_PREF;
  return CMD_SUCCESS;
}

ALIAS (no_bgp_default_local_preference,
       no_bgp_default_local_preference_val_cmd,
       "no bgp default local-preference <0-4294967295>",
       NO_STR
       "BGP specific commands\n"
       "Configure BGP defaults\n"
       "local preference (higher=more preferred)\n"
       "Configure default local preference value\n")

/* allocate new peer object */
struct peer *
peer_new ()
{
  struct peer *peer;
  struct servent *sp;

  /* Allocate new peer. */
  peer = XMALLOC (MTYPE_BGP_PEER, sizeof (struct peer));
  memset (peer, 0, sizeof (struct peer));

  /* Set default value. */
  peer->fd = -1;
  peer->v_start = BGP_INIT_START_TIMER;
  peer->v_connect = BGP_DEFAULT_CONNECT_RETRY;
  peer->v_holdtime = BGP_DEFAULT_HOLDTIME;
  peer->v_keepalive = BGP_DEFAULT_KEEPALIVE;
  peer->v_asorig = BGP_DEFAULT_ASORIGINATE;
  peer->v_routeadv = BGP_DEFAULT_ROUTEADV;
  peer->status = Idle;
  peer->ostatus = Idle;
  peer->version = BGP_VERSION_4;
  peer->weight = 0;
  peer->translate_update  = 0;
  SET_FLAG (peer->af_flags[AFI_IP][SAFI_UNICAST], PEER_FLAG_SEND_COMMUNITY);
  SET_FLAG (peer->af_flags[AFI_IP][SAFI_MULTICAST], PEER_FLAG_SEND_COMMUNITY);
  SET_FLAG (peer->af_flags[AFI_IP][SAFI_MPLS_VPN], PEER_FLAG_SEND_COMMUNITY);
  SET_FLAG (peer->af_flags[AFI_IP6][SAFI_UNICAST], PEER_FLAG_SEND_COMMUNITY);
  SET_FLAG (peer->af_flags[AFI_IP6][SAFI_MULTICAST], PEER_FLAG_SEND_COMMUNITY);
  SET_FLAG (peer->af_flags[AFI_IP][SAFI_UNICAST], PEER_FLAG_SEND_EXT_COMMUNITY);
  SET_FLAG (peer->af_flags[AFI_IP][SAFI_MULTICAST], PEER_FLAG_SEND_EXT_COMMUNITY);
  SET_FLAG (peer->af_flags[AFI_IP][SAFI_MPLS_VPN], PEER_FLAG_SEND_EXT_COMMUNITY);
  SET_FLAG (peer->af_flags[AFI_IP6][SAFI_UNICAST], PEER_FLAG_SEND_EXT_COMMUNITY);
  SET_FLAG (peer->af_flags[AFI_IP6][SAFI_MULTICAST], PEER_FLAG_SEND_EXT_COMMUNITY);
  SET_FLAG (peer->flags, PEER_FLAG_CAPABILITY_ROUTE_REFRESH);
  SET_FLAG (peer->sflags, PEER_STATUS_CAPABILITY_OPEN);

  peer->ibuf = stream_new (BGP_MAX_PACKET_SIZE);
  peer->obuf = stream_fifo_new ();
  peer->conf = list_new ();

  peer->adj_in[AFI_IP][SAFI_UNICAST] = route_table_init ();
  peer->adj_in[AFI_IP][SAFI_MULTICAST] = route_table_init ();
  peer->adj_in[AFI_IP6][SAFI_UNICAST] = route_table_init ();
  peer->adj_in[AFI_IP6][SAFI_MULTICAST] = route_table_init ();

  peer->adj_out[AFI_IP][SAFI_UNICAST] = route_table_init ();
  peer->adj_out[AFI_IP][SAFI_MULTICAST] = route_table_init ();
  peer->adj_out[AFI_IP][SAFI_MPLS_VPN] = route_table_init ();
  peer->adj_out[AFI_IP6][SAFI_UNICAST] = route_table_init ();
  peer->adj_out[AFI_IP6][SAFI_MULTICAST] = route_table_init ();

  /* Get service port number. */
  sp = getservbyname ("bgp", "tcp");
  peer->port = (sp == NULL) ? BGP_PORT_DEFAULT : ntohs(sp->s_port);

  return peer;
}

/* Check peer's AS number and determin is this peer IBPG or EBGP */
int
peer_sort (struct peer *peer)
{
  /* Find the relevant BGP structure */
  struct bgp *bgp;
  struct peer_conf *conf;
  struct listnode *nn;

  /* This becomes slightly more complicated as we have to find the
     CONFEDERATION list, so we can see if this is a BGP_PEER_CONFED */
  bgp = NULL;
  LIST_LOOP (peer->conf, conf, nn)
    {
      bgp = conf->bgp;
    }

  if(bgp && CHECK_FLAG(bgp->config, BGP_CONFIG_CONFEDERATION))
    {
      if (peer->local_as == 0)
	return BGP_PEER_INTERNAL;

      if (peer->local_as == peer->as)
	{
	  if (peer->local_as == bgp->confederation_id)
	    return BGP_PEER_EBGP;
	  else
	    return BGP_PEER_IBGP;
	}

      if (bgp_confederation_peers_check(bgp, peer->as))
	return BGP_PEER_CONFED;

      return BGP_PEER_EBGP;
    }
  else
    {
      return (peer->local_as == 0
	      ? BGP_PEER_INTERNAL : peer->local_as == peer->as
	      ? BGP_PEER_IBGP : BGP_PEER_EBGP);
    }
}

int
peer_list_cmp (struct peer *p1, struct peer *p2)
{
  return sockunion_cmp (&p1->su, &p2->su);
}

int
peer_conf_cmp (struct peer_conf *p1, struct peer_conf *p2)
{
  return sockunion_cmp (&p1->peer->su, &p2->peer->su);
}

struct peer_conf *
peer_conf_new()
{
  struct peer_conf *pconf;

  pconf = XMALLOC (MTYPE_PEER_CONF, sizeof (struct peer_conf));
  memset (pconf, 0, sizeof (struct peer_conf));
  return pconf;
}

void
peer_conf_free (struct peer_conf *pconf)
{
  XFREE (MTYPE_PEER_CONF, pconf);
}

void
peer_conf_delete (struct peer_conf *conf)
{
  int i;
  afi_t afi;
  safi_t safi;
  struct bgp_filter *filter;

  for (afi = AFI_IP; afi < AFI_MAX; afi++)
    for (safi = SAFI_UNICAST; safi < SAFI_MAX; safi++)
      {
	filter = &conf->filter[afi][safi];

	for (i = FILTER_IN; i < FILTER_MAX; i++)
	  {
	    if (filter->dlist[i].name)
	      free (filter->dlist[i].name);
	    if (filter->plist[i].name)
	      free (filter->plist[i].name);
	    if (filter->aslist[i].name)
	      free (filter->aslist[i].name);
	    if (filter->map[i].name)
	      free (filter->map[i].name);
	  }
      }
  peer_conf_free (conf);
}

/* BGP instance creation by `router bgp' commands. */
struct bgp *
bgp_create ()
{
  struct bgp *bgp;
  afi_t afi;
  safi_t safi;

  bgp = XMALLOC (MTYPE_BGP, sizeof (struct bgp));
  memset (bgp, 0, sizeof (struct bgp));

  bgp->peer_group = list_new ();
  bgp->peer_conf = list_new ();
  bgp->peer_conf->cmp = (int (*)(void *, void *)) peer_conf_cmp;

  for (afi = AFI_IP; afi < AFI_MAX; afi++)
    for (safi = SAFI_UNICAST; safi < SAFI_MAX; safi++)
      {
	bgp->route[afi][safi] = route_table_init ();
	bgp->aggregate[afi][safi] = route_table_init ();
	bgp->rib[afi][safi] = route_table_init ();
      }

  bgp->default_local_pref = BGP_DEFAULT_LOCAL_PREF;
  bgp->default_holdtime = BGP_DEFAULT_HOLDTIME;
  bgp->default_keepalive = BGP_DEFAULT_KEEPALIVE;

  return bgp;
}

/* Return first entry of BGP. */
struct bgp *
bgp_get_default ()
{
  if (bgp_list->head)
    return bgp_list->head->data;
  return NULL;
}

/* Lookup BGP entry. */
struct bgp *
bgp_lookup (as_t as, char *name)
{
  struct bgp *bgp;
  struct listnode *nn;

  LIST_LOOP (bgp_list, bgp, nn)
    if (bgp->as == as
	&& ((bgp->name == NULL && name == NULL) 
	    || (bgp->name && name && strcmp (bgp->name, name) == 0)))
      return bgp;
  return NULL;
}

/* Lookup BGP structure by view name. */
struct bgp *
bgp_lookup_by_name (char *name)
{
  struct bgp *bgp;
  struct listnode *nn;

  LIST_LOOP (bgp_list, bgp, nn)
    if ((bgp->name == NULL && name == NULL)
	|| (bgp->name && name && strcmp (bgp->name, name) == 0))
      return bgp;
  return NULL;
}

/* Called from VTY commands. */
int
bgp_get (struct vty *vty, as_t as, char *name)
{
  struct bgp *bgp;

  /* Multiple instance check. */
  if (! bgp_multiple_instance)
    {
      if (name)
	{
	  vty_out (vty, "Please specify 'bgp multiple-instance' first%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      /* Get first BGP structure if exists. */
      bgp = bgp_get_default ();

      if (bgp)
	{
	  if (bgp->as != as)
	    {
	      vty_out (vty, "BGP is already running; AS is %d%s", bgp->as,
		       VTY_NEWLINE);
	      return CMD_WARNING;
	    }
	  vty->node = BGP_NODE;
	  vty->index = bgp;
	  return CMD_SUCCESS;
	}

      bgp = bgp_create ();
      bgp->as = as;
      listnode_add (bgp_list, bgp);
      bgp_if_update_all ();
      vty->node = BGP_NODE;
      vty->index = bgp;
      return CMD_SUCCESS;
    }
  else
    {
      bgp = bgp_lookup (as, name);

      if (bgp)
	{
	  vty->node = BGP_NODE;
	  vty->index = bgp;
	  return CMD_SUCCESS;
	}
      
      bgp = bgp_create ();
      bgp->as = as;
      if (name)
	bgp->name = strdup (name);
      listnode_add (bgp_list, bgp);
      bgp_if_update_all ();
      vty->node = BGP_NODE;
      vty->index = bgp;

      return CMD_SUCCESS;
    }
  return CMD_SUCCESS;
}

int
bgp_get_by_str (struct vty *vty, char *as_str, char *name)
{
  char *endptr = NULL;
  as_t as;

  /* Convert string to number. */
  as = strtoul (as_str, &endptr, 10);
  if (as == ULONG_MAX || *endptr != '\0' || as < 1 || as > 65535)
    {
      vty_out (vty, "AS value error%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  return bgp_get (vty, as, name);
}

/* Delete BGP instance. */
void
bgp_delete (struct bgp *bgp)
{
  struct peer_conf *conf;
  struct listnode *nn;
  struct listnode *next;
  afi_t afi;
  safi_t safi;

  /* Delete static route. */
  bgp_static_delete (bgp);

  bgp->peer_group->del = (void (*)(void *)) peer_group_free;

  list_delete (bgp->peer_group);

  for (nn = bgp->peer_conf->head; nn; nn = next)
    {
      conf = nn->data;
      next = nn->next;
      peer_delete (conf->peer);
    }

  /* Clear peer_conf */
  list_delete (bgp->peer_conf);

  listnode_delete (bgp_list, bgp);

  if (bgp->name)
    free (bgp->name);
  
  for (afi = AFI_IP; afi < AFI_MAX; afi++)
    for (safi = SAFI_UNICAST; safi < SAFI_MAX; safi++)
      {
	if (bgp->route[afi][safi])
	  XFREE (MTYPE_ROUTE_TABLE, bgp->route[afi][safi]);
	if (bgp->aggregate[afi][safi])
	  XFREE (MTYPE_ROUTE_TABLE,bgp->aggregate[afi][safi]) ;
	if (bgp->rib[afi][safi])
	  XFREE (MTYPE_ROUTE_TABLE,bgp->rib[afi][safi]);
      }

  XFREE (MTYPE_BGP, bgp);
}

/* This function is called from VTY command.  Act as a wrapper of
   bgp_delte (). */
int
bgp_destroy (struct vty *vty, char *as_str, char *name)
{
  struct bgp *bgp;
  char *endptr = NULL;
  as_t as;
  /* struct in_addr id; */

  /* Convert string to number. */
  as = strtoul (as_str, &endptr, 10);
  if (as == ULONG_MAX || *endptr != '\0' || as < 1 || as > 65535)
    {
      vty_out (vty, "AS value error%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Lookup bgp structure. */
  bgp = bgp_lookup (as, name);

  if (!bgp)
    {
      vty_out (vty, "Can't find BGP instance%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  bgp_delete (bgp);

  return CMD_SUCCESS;
}

/* `router bgp' commands. */
DEFUN (router_bgp, 
       router_bgp_cmd, 
       "router bgp <1-65535>",
       ROUTER_STR
       BGP_STR
       AS_STR)
{
  return bgp_get_by_str (vty, argv[0], NULL);
}

DEFUN (router_bgp_view,
       router_bgp_view_cmd,
       "router bgp <1-65535> view WORD",
       ROUTER_STR
       BGP_STR
       AS_STR
       "BGP view\n"
       "view name\n")
{
  return bgp_get_by_str (vty, argv[0], argv[1]);
}

/* `no router bgp' commands. */
DEFUN (no_router_bgp,
       no_router_bgp_cmd,
       "no router bgp <1-65535>",
       NO_STR
       ROUTER_STR
       BGP_STR
       AS_STR)
{
  return bgp_destroy (vty, argv[0], NULL);
}

DEFUN (no_router_bgp_view,
       no_router_bgp_view_cmd,
       "no router bgp <1-65535> view WORD",
       NO_STR
       ROUTER_STR
       BGP_STR
       AS_STR
       "BGP view\n"
       "view name\n")
{
  return bgp_destroy (vty, argv[0], argv[1]);
}

/* Peer identification.

   Peer structure is identified by it's IP address, local AS number,
   remote AS number and local router-id.  Normally, local router-id
   identification is used only for Merit MRT like route server
   configuration.

   When user configure the peer under specific BGP instance node, only
   IP address and local AS number are used for looking up.  If the
   peer's remote AS number and user configuration AS number is
   different, the peer's AS number is changed. */

struct peer *
peer_lookup_with_local_as (union sockunion *su, as_t local_as)
{
  struct peer *peer;
  struct listnode *nn;

  LIST_LOOP (peer_list, peer, nn)
    {
      if (sockunion_same (&peer->su, su) 
	  && peer->local_as == local_as)
	return peer;
    }
  return NULL;
}

/* Accepting remote BGP connection, at least remote connection's
   source IP address is configured as a peer.  This function check the
   existance of the IP address. */

struct peer *
peer_lookup_by_su (union sockunion *su)
{
  struct peer *peer;
  struct listnode *nn;

  LIST_LOOP (peer_list, peer, nn)
    {
      if (sockunion_same (&peer->su, su)
	  && ! CHECK_FLAG (peer->sflags, PEER_STATUS_ACCEPT_PEER))
	return peer;
    }
  return NULL;
}

/* BGP Open packet includes remote router's AS number and router-id.
   We lookup local peer with those information.  First loop check
   exact match peer including remote router-id.  Second loop check
   anonymous router-id peer.  */

struct peer *
peer_lookup_with_open (union sockunion *su, as_t remote_as,
		       struct in_addr *remote_id, int *as)
{
  struct peer *peer;
  struct listnode *nn;

  LIST_LOOP (peer_list, peer, nn)
    {
      if (sockunion_same (&peer->su, su)
	  && ! CHECK_FLAG (peer->sflags, PEER_STATUS_ACCEPT_PEER))
	{
	  if (peer->as == remote_as
	      && peer->remote_id.s_addr == remote_id->s_addr)
	    return peer;
	  if (peer->as == remote_as)
	    *as = 1;
	}
    }
  LIST_LOOP (peer_list, peer, nn)
    {
      if (sockunion_same (&peer->su, su)
	  &&  ! CHECK_FLAG (peer->sflags, PEER_STATUS_ACCEPT_PEER))
	{
	  if (peer->as == remote_as
	      && peer->remote_id.s_addr == 0)
	    return peer;
	  if (peer->as == remote_as)
	    *as = 1;
	}
    }
  return NULL;
}

struct peer_conf *
peer_conf_lookup (struct bgp *bgp, union sockunion *su, int afi)
{
  struct listnode *nn;
  struct peer_conf *conf;

  LIST_LOOP (bgp->peer_conf, conf, nn)
    {
      if (sockunion_same (&conf->peer->su, su))
	return conf;
    }
  return NULL;
}

/* Utility function for lookup peer from VTY commands. */
struct peer_conf *
peer_conf_lookup_vty (struct vty *vty, char *ip_str, int afi)
{
  int ret;
  struct bgp *bgp;
  union sockunion su;
  struct peer_conf *conf;

  bgp = vty->index;

  ret = str2sockunion (ip_str, &su);
  if (ret < 0)
    {
      vty_out (vty, "Malformed address: %s%s", ip_str, VTY_NEWLINE);
      return NULL;
    }

  conf = peer_conf_lookup (bgp, &su, afi);
  if (! conf)
    {
      vty_out (vty, "Can't find peer: %s%s", ip_str, VTY_NEWLINE);
      return NULL;
    }
  return conf;
}

struct peer_conf *
peer_conf_lookup_existing (struct bgp *bgp, union sockunion *su)
{
  struct listnode *nn;
  struct peer_conf *conf;

  LIST_LOOP (bgp->peer_conf, conf, nn)
    {
      if (sockunion_same (&conf->peer->su, su))
	return conf;
    }
  return NULL;
}

#define BGP_UPTIME_LEN 25

/* Display peer uptime. */
char *
peer_uptime (time_t uptime2, char *buf, size_t len)
{
  time_t uptime1;
  struct tm *tm;

  /* Check buffer length. */
  if (len < BGP_UPTIME_LEN)
    {
      zlog_warn ("peer_uptime (): buffer shortage %d", len);
      return "";
    }

  /* If there is no connection has been done before print `never'. */
  if (uptime2 == 0)
    {
      snprintf (buf, len, "never   ");
      return buf;
    }

  /* Get current time. */
  uptime1 = time (NULL);
  uptime1 -= uptime2;
  tm = gmtime (&uptime1);

  /* Making formatted timer strings. */
#define ONE_DAY_SECOND 60*60*24
#define ONE_WEEK_SECOND 60*60*24*7

  if (uptime1 < ONE_DAY_SECOND)
    snprintf (buf, len, "%02d:%02d:%02d", 
	      tm->tm_hour, tm->tm_min, tm->tm_sec);
  else if (uptime1 < ONE_WEEK_SECOND)
    snprintf (buf, len, "%dd%02dh%02dm", 
	      tm->tm_yday, tm->tm_hour, tm->tm_min);
  else
    snprintf (buf, len, "%02dw%dd%02dh", 
	      tm->tm_yday/7, tm->tm_yday - ((tm->tm_yday/7) * 7), tm->tm_hour);
  return buf;
}

/* If peer is configured at least one address family return 1. */
int
peer_active (struct peer *peer)
{
  if (peer->afc[AFI_IP][SAFI_UNICAST]
      || peer->afc[AFI_IP][SAFI_MULTICAST]
      || peer->afc[AFI_IP][SAFI_MPLS_VPN]
      || peer->afc[AFI_IP6][SAFI_UNICAST]
      || peer->afc[AFI_IP6][SAFI_MULTICAST])
    return 1;
  return 0;
}

struct peer *
peer_create (union sockunion *su, as_t local_as, struct in_addr id,
	     as_t remote_as, u_int32_t holdtime, u_int32_t keepalive)
{
  struct peer *peer;
  char buf[SU_ADDRSTRLEN];

  peer = peer_new ();
  peer->su = *su;
  peer->local_as = local_as;
  peer->as = remote_as;
  peer->local_id = id;
  peer->global_holdtime = holdtime;
  peer->global_keepalive = keepalive;
  listnode_add_sort (peer_list, peer);

  /* Last read time set */
  peer->readtime = time(NULL);

  /* Default TTL set. */
  peer->ttl = (peer_sort (peer) == BGP_PEER_IBGP ? 255 : 1);

  /* Make peer's address string. */
  sockunion2str (su, buf, SU_ADDRSTRLEN);
  peer->host = strdup (buf);

  /* Set up peer's events and timers. */
  bgp_timer_set (peer);

  return peer;
}

/* Make accept BGP peer.  Called from bgp_accept (). */
struct peer *
peer_create_accept ()
{
  struct peer *peer;

  peer = peer_new ();
  listnode_add_sort (peer_list, peer);

  return peer;
}

/* Change peer's AS number */
int
peer_as_change (struct peer *peer, as_t as)
{
  /* Stop peer. */
  bgp_stop (peer);

  peer->as = as;

  /* ebgp-multihop reset. */

  return CMD_SUCCESS;
}

void
peer_af_flag_reset (afi_t afi, safi_t safi, struct peer_conf *conf)
{
  struct peer *peer;
  struct bgp_filter *filter;
  int i;

  peer = conf->peer;
  filter = &conf->filter[afi][safi];

  /* Clear neighbor filter and route-map */
  for (i = FILTER_IN; i < FILTER_MAX; i++)
    {
      if (filter->dlist[i].name)
	{
	  free (filter->dlist[i].name);
	  filter->dlist[i].name = NULL;
	}
      if (filter->plist[i].name)
	{
	  free (filter->plist[i].name);
	  filter->plist[i].name = NULL; 
	}
      if (filter->aslist[i].name)
	{
	  free (filter->aslist[i].name);
	  filter->aslist[i].name = NULL;
	}
      if (filter->map[i].name)
	{
	  free (filter->map[i].name);
	  filter->map[i].name = NULL;
	}
    }

  /* Clear neighbor next-hop-self */
  UNSET_FLAG (peer->af_flags[afi][safi], PEER_FLAG_NEXTHOP_SELF);

  /* Clear neighbor send-community */
  SET_FLAG (peer->af_flags[afi][safi], PEER_FLAG_SEND_COMMUNITY);
  SET_FLAG (peer->af_flags[afi][safi], PEER_FLAG_SEND_EXT_COMMUNITY);

  /* Clear neighbor route-reflector-client */
  UNSET_FLAG (peer->af_flags[afi][safi], PEER_FLAG_REFLECTOR_CLIENT);

  /* Clear neighbor route-server-client */
  UNSET_FLAG (peer->af_flags[afi][safi], PEER_FLAG_RSERVER_CLIENT);

  /* Clear neighbor soft-reconfiguration inbound */
  UNSET_FLAG (peer->af_flags[afi][safi], PEER_FLAG_SOFT_RECONFIG);

  /* Clear neighbor maximum-prefix */
  conf->pmax[afi][safi] = 0;
  conf->pmax_warning[afi][safi] =  0;
}

struct peer_conf *
peer_conf_create (int afi, int safi, struct peer *peer)
{
  struct peer_conf *conf;
  int active;

  /* Make new peer configuration then link it to the peer. */
  conf = peer_conf_new ();
  conf->peer = peer;
  listnode_add_sort (peer->conf, conf);

  /* Store peer's active status. */
  active = peer_active (peer);

  if (safi & SAFI_UNICAST)
    {
      conf->afc[afi][SAFI_UNICAST] = 1;
      peer->afc[afi][SAFI_UNICAST]++;
    }
  if (safi & SAFI_MULTICAST)
    {
      conf->afc[afi][SAFI_MULTICAST] = 1;
      peer->afc[afi][SAFI_MULTICAST]++;
    }
  if (safi == SAFI_MPLS_VPN)
    {
      conf->afc[afi][safi] = 1;
      peer->afc[afi][safi]++;
    }

  /* If this configuration activate the peer, set start timer. */
  if (! active && peer_active (peer))
    bgp_timer_set (peer);

  return conf;
}

void
peer_conf_active (int afi, int safi, struct peer_conf *conf)
{
  int active;
  struct peer *peer;

  peer = conf->peer;
  active = peer_active (peer);

  conf->afc[afi][safi] = 1;
  conf->peer->afc[afi][safi]++;

  /* If this configuration activate the peer, set start timer. */
  if (! active && peer_active (peer))
    bgp_timer_set (peer);
  else
    BGP_EVENT_ADD (peer, BGP_Stop);
}

void
peer_conf_deactive (int afi, int safi, struct peer_conf *conf)
{
  struct peer *peer;

  peer = conf->peer;

  /* Must be configured. */
  if (! conf->afc[afi][safi])
    return;

  conf->afc[afi][safi] = 0;
  peer->afc[afi][safi]--;

  /* Clear peer Per AF configuration */ 
  peer_af_flag_reset (afi, safi, conf);

  BGP_EVENT_ADD (peer, BGP_Stop);
}

/* Make or change remote peer's AS number. */
int
peer_remote_as (struct vty *vty, char *ip_str, char *as_str, int afi, int safi)
{
  int ret;
  struct bgp *bgp;
  char *endptr = NULL;
  as_t as;
  union sockunion su;
  struct peer *peer;
  struct peer_conf *conf;

  ret = str2sockunion (ip_str, &su);
  if (ret < 0)
    {
      vty_out (vty, "Malformed address: %s%s", ip_str, VTY_NEWLINE);
      return CMD_WARNING;
    }

  as = strtoul (as_str, &endptr, 10);
  if (as == ULONG_MAX || *endptr != '\0' || as < 1 || as > 65535)
    {
      vty_out (vty, "AS value error: %s%s", as_str, VTY_NEWLINE);
      return CMD_WARNING;
    }

  bgp = vty->index;

  peer = peer_lookup_with_local_as (&su, bgp->as);
  if (CHECK_FLAG(bgp->config, BGP_CONFIG_CONFEDERATION) && ! peer)
    {
      peer = peer_lookup_with_local_as (&su, bgp->confederation_id);
    }

  if (peer)
    {
      /* Lookup peer_conf */
      conf = peer_conf_lookup (bgp, &su, afi);

      if (! conf)
	{
	  /* New peer configuration. */
	  conf = peer_conf_create (afi, safi, peer);
	  conf->bgp = bgp;
	  listnode_add_sort (bgp->peer_conf, conf);
	}

      /* Existing peer's AS number change. */
      if (peer->as != as)
	peer_as_change (peer, as);

      /* Existing peer's SAFI change. */
      /* XXXX need code here. */;
    }
  else
    {
      /* Real peer creation. */

      /* If the peer is not part of our CONFED, and its not an iBGP peer then
	 spoof the source AS */
      if (CHECK_FLAG (bgp->config, BGP_CONFIG_CONFEDERATION) 
	  && ! bgp_confederation_peers_check(bgp, as) 
	  && bgp->as != as)
        {
          peer = peer_create (&su, bgp->confederation_id,
			      bgp->id, as, bgp->default_holdtime,
			      bgp->default_keepalive); 
        }
      else
	{
	  peer = peer_create (&su, bgp->as, bgp->id, as,
			      bgp->default_holdtime,
			      bgp->default_keepalive);
	}

      /* If this is IPv4 unicast configuration and "no bgp default
         ipv4-unicast" is specified. */
      if (CHECK_FLAG (bgp->config, BGP_CONFIG_NO_DEFAULT_IPV4)
	  && afi == AFI_IP && safi == SAFI_UNICAST)
	conf = peer_conf_create (0, 0, peer);
      else
	conf = peer_conf_create (afi, safi, peer);

      conf->bgp = bgp;
      listnode_add_sort (bgp->peer_conf, conf);
    }

  return CMD_SUCCESS;
}

int
peer_activate (struct vty *vty, char *ip_str, int afi, int safi)
{
  int ret;
  union sockunion su;
  struct bgp *bgp;
  struct peer_conf *conf;

  bgp = vty->index;

  /* Lookup peer. */
  ret = str2sockunion (ip_str, &su);
  if (ret < 0)
    {
      vty_out (vty, "Malformed address: %s%s", ip_str, VTY_NEWLINE);
      return CMD_WARNING;
    }

  conf = peer_conf_lookup_existing (bgp, &su);
  if (! conf)
    {
      vty_out (vty, "%% Specify remote-as or peer-group commands first%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Activate the address family configuration. */
  if (! conf->afc[afi][safi])
    peer_conf_active (afi, safi, conf);

  return CMD_SUCCESS;
}

int
peer_deactivate (struct vty *vty, char *ip_str, int afi, int safi)
{
  int ret;
  union sockunion su;
  struct bgp *bgp;
  struct peer_conf *conf;

  bgp = vty->index;

  /* Lookup peer. */
  ret = str2sockunion (ip_str, &su);
  if (ret < 0)
    {
      vty_out (vty, "Malformed address: %s%s", ip_str, VTY_NEWLINE);
      return CMD_WARNING;
    }

  conf = peer_conf_lookup_existing (bgp, &su);
  if (! conf)
    {
      vty_out (vty, "%% Specify remote-as or peer-group commands first%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* De-activate the address family configuration. */
  if (conf->afc[afi][safi])
    peer_conf_deactive (afi, safi, conf);

  return CMD_SUCCESS;
}

/* Delete peer from confguration. */
void
peer_delete (struct peer *peer)
{
  struct peer_conf *conf;
  struct listnode *nn;

  /* Withdraw all information from routing table.  We can not use
     BGP_EVENT_ADD (peer, BGP_Stop) at here.  Because the event is
     executed after peer structure is deleted. */
  bgp_stop (peer);
  fsm_change_status (peer, Idle);

  /* Delete peer_conf link from BGP structure. */
  LIST_LOOP (peer->conf, conf, nn)
    {
      listnode_delete (conf->bgp->peer_conf, conf);
    }

  /* Free peer_conf structure. */
  peer->conf->del = (void (*) (void *)) peer_conf_delete;
  list_delete (peer->conf);
  peer->conf = NULL;

  /* Stop all timers. */
  BGP_TIMER_OFF (peer->t_start);
  BGP_TIMER_OFF (peer->t_keepalive);
  BGP_TIMER_OFF (peer->t_holdtime);
  BGP_TIMER_OFF (peer->t_connect);
  BGP_TIMER_OFF (peer->t_asorig);
  BGP_TIMER_OFF (peer->t_routeadv);

  /* Delete from all peer list. */
  listnode_delete (peer_list, peer);

  if (peer->ibuf)
    stream_free (peer->ibuf);

  if (peer->obuf)
    stream_fifo_free (peer->obuf);

  /* Free allocated host character. */
  if (peer->host)
    free (peer->host);

  /* Local and remote addresses. */
  if (peer->su_local)
    XFREE (MTYPE_TMP, peer->su_local);
  if (peer->su_remote)
    XFREE (MTYPE_TMP, peer->su_remote);

  if (peer->adj_in[AFI_IP][SAFI_UNICAST])
    XFREE (MTYPE_ROUTE_TABLE,peer->adj_in [AFI_IP][SAFI_UNICAST]   );
  if (peer->adj_in[AFI_IP][SAFI_MULTICAST])
    XFREE (MTYPE_ROUTE_TABLE,peer->adj_in [AFI_IP][SAFI_MULTICAST] );
  if (peer->adj_in[AFI_IP6][SAFI_UNICAST])
    XFREE (MTYPE_ROUTE_TABLE,peer->adj_in [AFI_IP6][SAFI_UNICAST]  );
  if (peer->adj_in[AFI_IP6][SAFI_MULTICAST])
    XFREE (MTYPE_ROUTE_TABLE,peer->adj_in [AFI_IP6][SAFI_MULTICAST]);
  if (peer->adj_out[AFI_IP][SAFI_UNICAST])
    XFREE (MTYPE_ROUTE_TABLE,peer->adj_out[AFI_IP][SAFI_UNICAST]   );
  if (peer->adj_out[AFI_IP][SAFI_MULTICAST])
    XFREE (MTYPE_ROUTE_TABLE,peer->adj_out[AFI_IP][SAFI_MULTICAST] );
  if (peer->adj_out[AFI_IP][SAFI_MPLS_VPN])
    XFREE (MTYPE_ROUTE_TABLE,peer->adj_out[AFI_IP][SAFI_MPLS_VPN]  );
  if (peer->adj_out[AFI_IP6][SAFI_UNICAST])
    XFREE (MTYPE_ROUTE_TABLE,peer->adj_out[AFI_IP6][SAFI_UNICAST]  );
  if (peer->adj_out[AFI_IP6][SAFI_MULTICAST])
    XFREE (MTYPE_ROUTE_TABLE,peer->adj_out[AFI_IP6][SAFI_MULTICAST]);	    

  /* Free peer structure. */
  XFREE (MTYPE_BGP_PEER, peer);
}

int
peer_destroy (struct vty *vty, char *ip_str, char *as_str, int afi, int safi)
{
  int ret;
  struct bgp *bgp;
  char *endptr = NULL;
  as_t as = 0;
  union sockunion su;
  struct peer *peer;

  ret = str2sockunion (ip_str, &su);
  if (ret < 0)
    {
      vty_out (vty, "Malformed address: %s%s", ip_str, VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (as_str)
    {
      as = strtoul (as_str, &endptr, 10);
      if (as == ULONG_MAX || *endptr != '\0' || as < 1 || as > 65535)
	{
	  vty_out (vty, "AS value error: %s%s", as_str, VTY_NEWLINE);
	  return CMD_WARNING;
	}
    }

  bgp = vty->index;

  peer = peer_lookup_with_local_as (&su, bgp->as);
  if (CHECK_FLAG(bgp->config, BGP_CONFIG_CONFEDERATION) && !peer)
    {
      peer = peer_lookup_with_local_as (&su, bgp->confederation_id);
    }

  if (! peer)
    {
      vty_out (vty, "Can't find peer: %s%s", ip_str, VTY_NEWLINE);
      return CMD_WARNING;
    }
  if (as_str && peer->as != as)
    {
      vty_out (vty, "AS mismatch%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  peer_delete (peer);

  return CMD_SUCCESS;
}

/* peer_flag_change_type. */
enum flag_change_type
{
  flag_change_set,
  flag_change_set_reset,
  flag_change_unset,
  flag_change_unset_reset
};

/* Change specified peer flag. */
int
peer_change_af_flag (struct vty *vty, char *ip_str, afi_t afi, safi_t safi,
		     u_int16_t flag, enum flag_change_type type)
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  if (! conf->afc[afi][safi])
    {
      vty_out (vty, "%% Activate the neighbor for the address family first%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  peer = conf->peer;
 
  if (type == flag_change_set
      || type == flag_change_set_reset)
    {
      if (CHECK_FLAG (peer->af_flags[afi][safi], flag))
	return CMD_WARNING;

      SET_FLAG (peer->af_flags[afi][safi], flag);
      if (type == flag_change_set_reset)
	BGP_EVENT_ADD (peer, BGP_Stop);
    }
  else
    {
      if (! CHECK_FLAG (peer->af_flags[afi][safi], flag))
	return CMD_WARNING;

      UNSET_FLAG (peer->af_flags[afi][safi], flag);
      if (type == flag_change_unset_reset)
	BGP_EVENT_ADD (peer, BGP_Stop);
    }

  return CMD_SUCCESS;
}

/* Change specified peer flag. */
int
peer_change_flag (struct vty *vty, char *ip_str, int afi, u_int16_t flag,
		  int set)
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  peer = conf->peer;
  
  if (set)
    SET_FLAG (peer->flags, flag);
  else
    UNSET_FLAG (peer->flags, flag);
  return CMD_SUCCESS;
}

/* Change specified peer flag with resetting the connection.  If the
   flag is not changed nothing occur. */
int
peer_change_flag_with_reset (struct vty *vty, char *ip_str, int afi,
			     u_int16_t flag, int set)
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  peer = conf->peer;

  if (set)
    {
      if (! CHECK_FLAG (peer->flags, flag))
	{
	  SET_FLAG (peer->flags, flag);
	  BGP_EVENT_ADD (peer, BGP_Stop);
	}
    }
  else
    {
      if (CHECK_FLAG (peer->flags, flag))
	{
	  UNSET_FLAG (peer->flags, flag);
	  BGP_EVENT_ADD (peer, BGP_Stop);
	}
    }
  return CMD_SUCCESS;
}

DEFUN (neighbor_remote_as,
       neighbor_remote_as_cmd,
       NEIGHBOR_CMD "remote-as <1-65535>",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Specify a BGP neighbor\n"
       AS_STR)
{
  return peer_remote_as (vty, argv[0], argv[1], AFI_IP, SAFI_UNICAST);
}

DEFUN (neighbor_remote_as_unicast,
       neighbor_remote_as_unicast_cmd,
       NEIGHBOR_CMD "remote-as <1-65535> nlri unicast",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Specify a BGP neighbor\n"
       AS_STR
       "Network Layer Reachable Information\n"
       "Configure for unicast routes\n")
{
  return peer_remote_as (vty, argv[0], argv[1], AFI_IP, SAFI_UNICAST);
}

DEFUN (neighbor_remote_as_multicast,
       neighbor_remote_as_multicast_cmd,
       NEIGHBOR_CMD "remote-as <1-65535> nlri multicast",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Specify a BGP neighbor\n"
       AS_STR
       "Network Layer Reachable Information\n"
       "Configure for multicast routes\n")
{
  return peer_remote_as (vty, argv[0], argv[1], AFI_IP, SAFI_MULTICAST);
}

DEFUN (neighbor_remote_as_unicast_multicast,
       neighbor_remote_as_unicast_multicast_cmd,
       NEIGHBOR_CMD "remote-as <1-65535> nlri unicast multicast",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Specify a BGP neighbor\n"
       AS_STR
       "Network Layer Reachable Information\n"
       "Configure for unicast routes\n"
       "Configure for multicast routes\n")
{
  return peer_remote_as (vty, argv[0], argv[1], AFI_IP, SAFI_UNICAST_MULTICAST);
}

DEFUN (neighbor_activate,
       neighbor_activate_cmd,
       NEIGHBOR_CMD "activate",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Enable the Address Family for this Neighbor\n")
{
  return peer_activate (vty, argv[0], bgp_node_afi (vty),
			bgp_node_safi (vty));
}

DEFUN (no_neighbor_activate,
       no_neighbor_activate_cmd,
       NO_NEIGHBOR_CMD "activate",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Enable the Address Family for this Neighbor\n")
{
  return peer_deactivate (vty, argv[0], bgp_node_afi (vty),
			  bgp_node_safi (vty));
}

#ifdef HAVE_IPV6
DEFUN (ipv6_bgp_neighbor_remote_as, 
       ipv6_bgp_neighbor_remote_as_cmd, 
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) remote-as <1-65535>",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Specify a BGP neighbor\n"
       AS_STR)
{
  return peer_remote_as (vty, argv[0], argv[1], AFI_IP6, SAFI_UNICAST);
}

DEFUN (ipv6_bgp_neighbor_remote_as_unicast, 
       ipv6_bgp_neighbor_remote_as_unicast_cmd, 
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) remote-as <1-65535> nlri unicast",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Specify a BGP neighbor\n"
       AS_STR
       "Network Layer Reachable Information\n"
       "Configure for unicast routes\n")
{
  return peer_remote_as (vty, argv[0], argv[1], AFI_IP6, SAFI_UNICAST);
}

DEFUN (ipv6_bgp_neighbor_remote_as_multicast, 
       ipv6_bgp_neighbor_remote_as_multicast_cmd, 
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) remote-as <1-65535> nlri multicast",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Specify a BGP neighbor\n"
       AS_STR
       "Network Layer Reachable Information\n"
       "Configure for multicast routes\n")
{
  return peer_remote_as (vty, argv[0], argv[1], AFI_IP6, SAFI_MULTICAST);
}

DEFUN (ipv6_bgp_neighbor_remote_as_unicast_multicast, 
       ipv6_bgp_neighbor_remote_as_unicast_multicast_cmd, 
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) remote-as <1-65535> nlri unicast multicast",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Specify a BGP neighbor\n"
       AS_STR
       "Network Layer Reachable Information\n"
       "Configure for unicast routes\n"
       "Configure for multicast routes\n")
{
  return peer_remote_as (vty, argv[0], argv[1], AFI_IP6, SAFI_UNICAST_MULTICAST);
}

#endif /* HAVE_IPV6 */

DEFUN (no_neighbor,
       no_neighbor_cmd,
       NO_NEIGHBOR_CMD,
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR)
{
  return peer_destroy (vty, argv[0], NULL, AFI_IP, SAFI_UNICAST);
}

DEFUN (no_neighbor_remote_as,
       no_neighbor_remote_as_cmd,
       NO_NEIGHBOR_CMD "remote-as <1-65535>",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Specify a BGP neighbor\n"
       AS_STR)
{
  return peer_destroy (vty, argv[0], argv[1], AFI_IP, SAFI_UNICAST);
}

DEFUN (no_ipv6_bgp_neighbor,
       no_ipv6_bgp_neighbor_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X)",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP Address\n"
       "IPv6 Address\n")
{
  return peer_destroy (vty, argv[0], NULL, AFI_IP6, SAFI_UNICAST);
}

DEFUN (no_ipv6_bgp_neighbor_remote_as,
       no_ipv6_bgp_neighbor_remote_as_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) remote-as <1-65535>",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP Address\n"
       "IPv6 Address\n"
       "Specify a BGP neighbor\n"
       AS_STR)
{
  return peer_destroy (vty, argv[0], argv[1], AFI_IP6, SAFI_UNICAST);
}

/* router-id set. */
int
peer_router_id (struct vty *vty, char *ip_str, int afi, char *id_str)
{
  struct peer *peer;
  struct peer_conf *conf;
  struct in_addr id;
  int ret;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;

  peer = conf->peer;

  if (id_str)
    {
      ret = inet_aton (id_str, &id);
      if (! ret)
	{
	  vty_out (vty, "Malformed router identifier: %s%s", id_str,
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}
      peer->remote_id = id;
    }
  else
    {
      peer->remote_id.s_addr = 0;
    }

  return CMD_SUCCESS;
}

DEFUN (neighbor_router_id,
       neighbor_router_id_cmd,
       NEIGHBOR_CMD "router-id A.B.C.D",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Set neighbor's special router-id value\n"
       "IP address\n")
{
  return peer_router_id (vty, argv[0], AFI_IP, argv[1]);
}

DEFUN (no_neighbor_router_id,
       no_neighbor_router_id_cmd,
       NO_NEIGHBOR_CMD "router-id A.B.C.D",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Set neighbor's special router-id value\n"
       "IP address\n")
{
  return peer_router_id (vty, argv[0], AFI_IP, NULL);
}

/* neighbor passive. */
DEFUN (neighbor_passive,
       neighbor_passive_cmd,
       NEIGHBOR_CMD "passive",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Don't send open messages to this neighbor\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP,
				      PEER_FLAG_PASSIVE, 1);
}

DEFUN (no_neighbor_passive,
       no_neighbor_passive_cmd,
       NO_NEIGHBOR_CMD "passive",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Don't send open messages to this neighbor\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP,
				      PEER_FLAG_PASSIVE, 0);
}

DEFUN (ipv6_bgp_neighbor_passive,
       ipv6_bgp_neighbor_passive_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) passive",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Don't send open messages to this neighbor\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP6,
				      PEER_FLAG_PASSIVE, 1);
}

DEFUN (no_ipv6_bgp_neighbor_passive,
       no_ipv6_bgp_neighbor_passive_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) passive",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Don't send open messages to this neighbor\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP6,
				      PEER_FLAG_PASSIVE, 0);
}

/* neighbor shutdown. */
DEFUN (neighbor_shutdown,
       neighbor_shutdown_cmd,
       NEIGHBOR_CMD "shutdown",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Administratively shut down this neighbor\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP,
				      PEER_FLAG_SHUTDOWN, 1);
}

DEFUN (no_neighbor_shutdown,
       no_neighbor_shutdown_cmd,
       NO_NEIGHBOR_CMD "shutdown",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Administratively shut down this neighbor\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP,
				      PEER_FLAG_SHUTDOWN, 0);
}

DEFUN (ipv6_bgp_neighbor_shutdown,
       ipv6_bgp_neighbor_shutdown_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) shutdown",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Administratively shut down this neighbor\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP6,
				      PEER_FLAG_SHUTDOWN, 1);
}

DEFUN (no_ipv6_bgp_neighbor_shutdown,
       no_ipv6_bgp_neighbor_shutdown_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) shutdown",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Administratively shut down this neighbor\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP6,
				      PEER_FLAG_SHUTDOWN, 0);
}

/* neighbor ebgp-multihop. */
int
peer_ebgp_multihop_set (struct vty *vty, char *ip_str, char *ttl_str, int afi)
{
  struct peer *peer;
  struct peer_conf *conf;
  int ttl = TTL_MAX;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;

  if (ttl_str)
    ttl = atoi (ttl_str);

  if (ttl == 0)
    {
      vty_out (vty, "TTL value error: %s%s", ttl_str, VTY_NEWLINE);
      return CMD_WARNING;
    }

  peer = conf->peer;
  if (peer_sort (peer) == BGP_PEER_IBGP)
    {
      vty_out (vty, "peer is IBGP peer%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  peer->ttl = ttl;

  /* Set runnning connection's ttl. */
  if (peer->fd >= 0)
    sockopt_ttl (peer->su.sa.sa_family, peer->fd, peer->ttl);

  return CMD_SUCCESS;
}

int
peer_ebgp_multihop_unset (struct vty *vty, char *ip_str, char *ttl_str,
			  int afi)
{
  struct peer *peer;
  struct peer_conf *conf;
  int ttl = TTL_MAX;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;

  if (ttl_str)
    ttl = atoi (ttl_str);

  if (ttl == 0)
    {
      vty_out (vty, "TTL value error: %s%s", ttl_str, VTY_NEWLINE);
      return CMD_WARNING;
    }

  peer = conf->peer;
  if (peer_sort (peer) == BGP_PEER_IBGP)
    {
      vty_out (vty, "peer is IBGP peer%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Set default EBGP TTL. */
  peer->ttl = 1;

  /* Set runnning connection's ttl. */
  if (peer->fd >= 0)
    sockopt_ttl (peer->su.sa.sa_family, peer->fd, peer->ttl);

  return CMD_SUCCESS;
}

/* neighbor ebgp-multihop. */
DEFUN (neighbor_ebgp_multihop,
       neighbor_ebgp_multihop_cmd,
       NEIGHBOR_CMD "ebgp-multihop",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Allow EBGP neighbors not on directly connected networks\n")
{
  return peer_ebgp_multihop_set (vty, argv[0], NULL, AFI_IP);
}

DEFUN (neighbor_ebgp_multihop_ttl,
       neighbor_ebgp_multihop_ttl_cmd,
       NEIGHBOR_CMD "ebgp-multihop <1-255>",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Allow EBGP neighbors not on directly connected networks\n"
       "maximum hop count\n")
{
  return peer_ebgp_multihop_set (vty, argv[0], argv[1], AFI_IP);
}

DEFUN (no_neighbor_ebgp_multihop,
       no_neighbor_ebgp_multihop_cmd,
       NO_NEIGHBOR_CMD "ebgp-multihop",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Allow EBGP neighbors not on directly connected networks\n")
{
  return peer_ebgp_multihop_unset (vty, argv[0], NULL, AFI_IP);
}

DEFUN (no_neighbor_ebgp_multihop_ttl,
       no_neighbor_ebgp_multihop_ttl_cmd,
       NO_NEIGHBOR_CMD "ebgp-multihop <1-255>",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Allow EBGP neighbors not on directly connected networks\n"
       "maximum hop count\n")
{
  return peer_ebgp_multihop_unset (vty, argv[0], argv[1], AFI_IP);
}

DEFUN (ipv6_bgp_neighbor_ebgp_multihop,
       ipv6_bgp_neighbor_ebgp_multihop_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) ebgp-multihop",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Allow EBGP neighbors not on directly connected networks\n")
{
  return peer_ebgp_multihop_set (vty, argv[0], NULL, AFI_IP6);
}

DEFUN (ipv6_bgp_neighbor_ebgp_multihop_ttl,
       ipv6_bgp_neighbor_ebgp_multihop_ttl_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) ebgp-multihop <1-255>",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Allow EBGP neighbors not on directly connected networks\n"
       "maximum hop count\n")
{
  return peer_ebgp_multihop_set (vty, argv[0], argv[1], AFI_IP6);
}

DEFUN (no_ipv6_bgp_neighbor_ebgp_multihop,
       no_ipv6_bgp_neighbor_ebgp_multihop_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) ebgp-multihop",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Allow EBGP neighbors not on directly connected networks\n")
{
  return peer_ebgp_multihop_unset (vty, argv[0], NULL, AFI_IP6);
}

DEFUN (no_ipv6_bgp_neighbor_ebgp_multihop_ttl,
       no_ipv6_bgp_neighbor_ebgp_multihop_ttl_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) ebgp-multihop <1-255>",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Allow EBGP neighbors not on directly connected networks\n"
       "maximum hop count\n")
{
  return peer_ebgp_multihop_unset (vty, argv[0], argv[1], AFI_IP6);
}

/* neighbor description. */
int
peer_description_set (struct vty *vty, char *ip_str, int afi, char *str)
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  peer = conf->peer;

  if (peer->desc)
    XFREE (MTYPE_TMP, peer->desc);
  peer->desc = str;
  return CMD_SUCCESS;
}

int
peer_description_unset (struct vty *vty, char *ip_str, int afi)
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  peer = conf->peer;

  if (peer->desc)
    XFREE (MTYPE_TMP, peer->desc);
  peer->desc = NULL;
  return CMD_SUCCESS;
}

DEFUN (neighbor_description,
       neighbor_description_cmd,
       NEIGHBOR_CMD "description .LINE",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Neighbor specific description\n"
       "Up to 80 characters describing this neighbor\n")
{
  int i;
  struct buffer *b;
  char *str;

  if (argc == 1)
    return CMD_SUCCESS;

  /* Make string from buffer.  This function should be provided by
     buffer.c. */
  b = buffer_new (1024);
  for (i = 1; i < argc; i++)
    {
      buffer_putstr (b, (u_char *)argv[i]);
      buffer_putc (b, ' ');
    }
  buffer_putc (b, '\0');
  str = buffer_getstr (b);
  buffer_free (b);

  return peer_description_set (vty, argv[0], AFI_IP, str);
}

DEFUN (no_neighbor_description,
       no_neighbor_description_cmd,
       NO_NEIGHBOR_CMD "description",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Neighbor specific description\n")
{
  return peer_description_unset (vty, argv[0], AFI_IP);
}

ALIAS (no_neighbor_description,
       no_neighbor_description_val_cmd,
       NO_NEIGHBOR_CMD "description .LINE",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Neighbor specific description\n"
       "Up to 80 characters describing this neighbor\n")

DEFUN (ipv6_bgp_neighbor_description,
       ipv6_bgp_neighbor_description_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) description .LINE",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Neighbor specific description\n"
       "Up to 80 characters describing this neighbor\n")
{
  int i;
  struct buffer *b;
  char *str;

  if (argc == 1)
    return CMD_SUCCESS;

  b = buffer_new (1024);
  for (i = 1; i < argc; i++)
    {
      buffer_putstr (b, (u_char *)argv[i]);
      buffer_putc (b, ' ');
    }
  buffer_putc (b, '\0');
  str = buffer_getstr (b);
  buffer_free (b);

  return peer_description_set (vty, argv[0], AFI_IP6, str);
}

DEFUN (no_ipv6_bgp_neighbor_description,
       no_ipv6_bgp_neighbor_description_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) description",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Neighbor specific description\n")
{
  return peer_description_unset (vty, argv[0], AFI_IP6);
}

ALIAS (no_ipv6_bgp_neighbor_description,
       no_ipv6_bgp_neighbor_description_val_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) description .LINE",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Neighbor specific description\n"
       "Up to 80 characters describing this neighbor\n")

/* neighbor next-hop-self. */
DEFUN (neighbor_nexthop_self,
       neighbor_nexthop_self_cmd,
       NEIGHBOR_CMD "next-hop-self",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Disable the next hop calculation for this neighbor\n")
{
  return peer_change_af_flag (vty, argv[0], bgp_node_afi (vty),
			      bgp_node_safi (vty), PEER_FLAG_NEXTHOP_SELF,
			      flag_change_set);
}

DEFUN (no_neighbor_nexthop_self,
       no_neighbor_nexthop_self_cmd,
       NO_NEIGHBOR_CMD "next-hop-self",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Disable the next hop calculation for this neighbor\n")
{
  return peer_change_af_flag (vty, argv[0], bgp_node_afi (vty),
			      bgp_node_safi (vty), PEER_FLAG_NEXTHOP_SELF,
			      flag_change_unset);
}

DEFUN (ipv6_bgp_neighbor_nexthop_self,
       ipv6_bgp_neighbor_nexthop_self_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) next-hop-self",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Disable the next hop calculation for this neighbor\n")
{
  return peer_change_af_flag (vty, argv[0], AFI_IP6, SAFI_UNICAST, 
			   PEER_FLAG_NEXTHOP_SELF, flag_change_set);
}

DEFUN (no_ipv6_bgp_neighbor_nexthop_self,
       no_ipv6_bgp_neighbor_nexthop_self_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) next-hop-self",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Disable the next hop calculation for this neighbor\n")
{
  return peer_change_af_flag (vty, argv[0], AFI_IP6, SAFI_UNICAST,
			   PEER_FLAG_NEXTHOP_SELF, flag_change_unset);
}

/* neighbor update-source. */
int
peer_update_source_set (struct vty *vty, char *ip_str, int afi, 
			char *source_str)
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  peer = conf->peer;

  if (peer->update_source)
    {
      XFREE (MTYPE_SOCKUNION, peer->update_source);
      peer->update_source = NULL;
    }
  if (peer->update_if)
    {
      free (peer->update_if);
      peer->update_if = NULL;
    }

  peer->update_source = sockunion_str2su (source_str);

  if (peer->update_source == NULL)
    peer->update_if = strdup (source_str);

  return CMD_SUCCESS;
}

int
peer_update_source_unset (struct vty *vty, char *ip_str, int afi)
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  peer = conf->peer;

  if (peer->update_source)
    {
      XFREE (MTYPE_SOCKUNION, peer->update_source);
      peer->update_source = NULL;
    }
  if (peer->update_if)
    {
      free (peer->update_if);
      peer->update_if = NULL;
    }

  return CMD_SUCCESS;
}

DEFUN (neighbor_update_source,
       neighbor_update_source_cmd,
       NEIGHBOR_CMD "update-source WORD",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Source of routing updates\n"
       "Interface name\n")
{
  return peer_update_source_set (vty, argv[0], AFI_IP, argv[1]);
}

DEFUN (no_neighbor_update_source,
       no_neighbor_update_source_cmd,
       NO_NEIGHBOR_CMD "update-source",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Source of routing updates\n"
       "Interface name\n")
{
  return peer_update_source_unset (vty, argv[0], AFI_IP);
}

DEFUN (ipv6_bgp_neighbor_update_source,
       ipv6_bgp_neighbor_update_source_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) update-source WORD",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Source of routing updates\n"
       "Interface name\n")
{
  return peer_update_source_set (vty, argv[0], AFI_IP6, argv[1]);
}

DEFUN (no_ipv6_bgp_neighbor_update_source,
       no_ipv6_bgp_neighbor_update_source_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) update-source",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Source of routing updates\n"
       "Interface name\n")
{
  return peer_update_source_unset (vty, argv[0], AFI_IP6);
}

/* neighbor default-originate. */
DEFUN (neighbor_default_originate,
       neighbor_default_originate_cmd,
       NEIGHBOR_CMD "default-originate",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Originate default route to this neighbor\n")
{
  return peer_change_flag (vty, argv[0], AFI_IP, PEER_FLAG_DEFAULT_ORIGINATE, 1);
}

DEFUN (no_neighbor_default_originate,
       no_neighbor_default_originate_cmd,
       NO_NEIGHBOR_CMD "default-originate",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Originate default route to this neighbor\n")
{
  return peer_change_flag (vty, argv[0], AFI_IP, PEER_FLAG_DEFAULT_ORIGINATE, 0);
}

DEFUN (ipv6_bgp_neighbor_default_originate,
       ipv6_bgp_neighbor_default_originate_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) default-originate",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Originate default route to this neighbor\n")
{
  return peer_change_flag (vty, argv[0], AFI_IP6, PEER_FLAG_DEFAULT_ORIGINATE, 1);
}

DEFUN (no_ipv6_bgp_neighbor_default_originate,
       no_ipv6_bgp_neighbor_default_originate_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) default-originate",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Originate default route to this neighbor\n")
{
  return peer_change_flag (vty, argv[0], AFI_IP6, PEER_FLAG_DEFAULT_ORIGINATE, 0);
}

/* neighbor port. */
int
peer_port (struct vty *vty, char *ip_str, int afi, char *port_str)
{
  struct peer *peer;
  struct peer_conf *conf;
  unsigned long port = 0;
  char *endptr = NULL;
  struct servent *sp;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  peer = conf->peer;

  if (port_str == NULL)
    { 
      sp = getservbyname ("bgp", "tcp");
      peer->port = (sp == NULL) ? BGP_PORT_DEFAULT : ntohs (sp->s_port);
    }
  else
    {
      port = strtoul (port_str, &endptr, 10);
      if (port == ULONG_MAX || *endptr != '\0')
	{
	  vty_out (vty, "port value error%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}
      if (port > 65535)
	{
	  vty_out (vty, "port value error%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}
    }
  /* Set peer port. */
  peer->port = port;

  return CMD_SUCCESS;
}

/* Set specified peer's BGP version.  */
DEFUN (neighbor_port,
       neighbor_port_cmd,
       NEIGHBOR_CMD "port <0-65535>",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Neighbor's BGP port\n"
       "TCP port number\n")
{
  return peer_port (vty, argv[0], AFI_IP, argv[1]);
}

DEFUN (no_neighbor_port,
       no_neighbor_port_cmd,
       NO_NEIGHBOR_CMD "port",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Neighbor's BGP port\n")
{
  return peer_port (vty, argv[0], AFI_IP, NULL);
}

ALIAS (no_neighbor_port,
       no_neighbor_port_val_cmd,
       NO_NEIGHBOR_CMD "port <0-65535>",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Neighbor's BGP port\n"
       "TCP port number\n")

DEFUN (ipv6_bgp_neighbor_port,
       ipv6_bgp_neighbor_port_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) port <0-65535>",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Neighbor's BGP port\n"
       "TCP port number\n")
{
  return peer_port (vty, argv[0], AFI_IP6, argv[1]);
}

DEFUN (no_ipv6_bgp_neighbor_port,
       no_ipv6_bgp_neighbor_port_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) port",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Neighbor's BGP port\n")
{
  return peer_port (vty, argv[0], AFI_IP6, NULL);
}

ALIAS (no_ipv6_bgp_neighbor_port,
       no_ipv6_bgp_neighbor_port_val_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) port <0-65535>",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Neighbor's BGP port\n"
       "TCP port number\n")

/* neighbor send-community. */
DEFUN (neighbor_send_community,
       neighbor_send_community_cmd,
       NEIGHBOR_CMD "send-community",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Send Community attribute to this neighbor (default enable)\n")
{
  return peer_change_af_flag (vty, argv[0], bgp_node_afi (vty), bgp_node_safi (vty),
			      PEER_FLAG_SEND_COMMUNITY, flag_change_set);
}

DEFUN (no_neighbor_send_community,
       no_neighbor_send_community_cmd,
       NO_NEIGHBOR_CMD "send-community",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Send Community attribute to this neighbor (default enable)\n")
{
  return peer_change_af_flag (vty, argv[0], bgp_node_afi (vty), bgp_node_safi (vty),
			      PEER_FLAG_SEND_COMMUNITY, flag_change_unset);
}

DEFUN (ipv6_bgp_neighbor_send_community,
       ipv6_bgp_neighbor_send_community_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) send-community",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Send Community attribute to this neighbor (default enable)\n")
{
  return peer_change_af_flag (vty, argv[0], AFI_IP6, SAFI_UNICAST,
			      PEER_FLAG_SEND_COMMUNITY, flag_change_set);
}

DEFUN (no_ipv6_bgp_neighbor_send_community,
       no_ipv6_bgp_neighbor_send_community_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) send-community",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Send Community attribute to this neighbor (default enable)\n")
{
  return peer_change_af_flag (vty, argv[0], AFI_IP6, SAFI_UNICAST,
			      PEER_FLAG_SEND_COMMUNITY, flag_change_unset);
}

/* neighbor send-community extended. */
DEFUN (neighbor_send_community_type,
       neighbor_send_community_type_cmd,
       NEIGHBOR_CMD "send-community (both|extended|standard)",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Send Community attribute to this neighbor (default enable)\n"
       "Send Standard and Extended Community attributes\n"
       "Send Extended Community attributes\n"
       "Send Standard Community attributes\n")
{
  int ret;

  if (strncmp (argv[1], "s", 1) == 0)
    return peer_change_af_flag (vty, argv[0], bgp_node_afi (vty), bgp_node_safi (vty),
				PEER_FLAG_SEND_COMMUNITY, flag_change_set);
  if (strncmp (argv[1], "e", 1) == 0)
    return peer_change_af_flag (vty, argv[0], bgp_node_afi (vty), bgp_node_safi (vty),
				PEER_FLAG_SEND_EXT_COMMUNITY, flag_change_set);

  ret = peer_change_af_flag (vty, argv[0], bgp_node_afi (vty), bgp_node_safi (vty),
			     PEER_FLAG_SEND_COMMUNITY, flag_change_set);
  if (ret == CMD_WARNING)
    return CMD_WARNING;
  ret = peer_change_af_flag (vty, argv[0], bgp_node_afi (vty), bgp_node_safi (vty),
			     PEER_FLAG_SEND_EXT_COMMUNITY, flag_change_set);
  if (ret == CMD_WARNING)
    return CMD_WARNING;

  return CMD_SUCCESS;
}

DEFUN (no_neighbor_send_community_type,
       no_neighbor_send_community_type_cmd,
       NO_NEIGHBOR_CMD "send-community (both|extended|standard)",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Send Community attribute to this neighbor (default enable)\n"
       "Send Standard and Extended Community attributes\n"
       "Send Extended Community attributes\n"
       "Send Standard Community attributes\n")
{
  int ret;

  if (strncmp (argv[1], "s", 1) == 0)
    return peer_change_af_flag (vty, argv[0], bgp_node_afi (vty), bgp_node_safi (vty),
				PEER_FLAG_SEND_COMMUNITY, flag_change_unset);
  if (strncmp (argv[1], "e", 1) == 0)
    return peer_change_af_flag (vty, argv[0], bgp_node_afi (vty), bgp_node_safi (vty),
				PEER_FLAG_SEND_EXT_COMMUNITY, flag_change_unset);

  ret = peer_change_af_flag (vty, argv[0], bgp_node_afi (vty), bgp_node_safi (vty),
			     PEER_FLAG_SEND_COMMUNITY, flag_change_unset);
  if (ret == CMD_WARNING)
    return CMD_WARNING;
  ret = peer_change_af_flag (vty, argv[0], bgp_node_afi (vty), bgp_node_safi (vty),
			     PEER_FLAG_SEND_EXT_COMMUNITY, flag_change_unset);
  if (ret == CMD_WARNING)
    return CMD_WARNING;

  return CMD_SUCCESS;
}

DEFUN (ipv6_bgp_neighbor_send_community_extended,
       ipv6_bgp_neighbor_send_community_extended_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) send-community extended",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Send Community attribute to this neighbor (default enable)\n"
       "Extended Community\n")
{
  return peer_change_af_flag (vty, argv[0], AFI_IP6, SAFI_UNICAST,
			      PEER_FLAG_SEND_EXT_COMMUNITY, flag_change_set);
}

DEFUN (no_ipv6_bgp_neighbor_send_community_extended,
       no_ipv6_bgp_neighbor_send_community_extended_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) send-community extended",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Send Community attribute to this neighbor (default enable)\n"
       "Extended Community\n")
{
  return peer_change_af_flag (vty, argv[0], AFI_IP6, SAFI_UNICAST,
			      PEER_FLAG_SEND_EXT_COMMUNITY, flag_change_unset);
}

/* neighbor weight. */
int
peer_weight_set (struct vty *vty, char *ip_str, int afi, char *weight_str)
{
  struct peer *peer;
  struct peer_conf *conf;
  unsigned long weight;
  char *endptr = NULL;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  peer = conf->peer;

  weight = strtoul (weight_str, &endptr, 10);
  if (weight == ULONG_MAX || *endptr != '\0')
    {
      vty_out (vty, "weight value error%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  if (weight > 65535)
    {
      vty_out (vty, "weight value error%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  
  /* Set weight flag to peer configure. */
  peer->weight = weight;

  return CMD_SUCCESS;
}

int
peer_weight_unset (struct vty *vty, char *ip_str, int afi)
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  peer = conf->peer;
  peer->weight = 0;

  return CMD_SUCCESS;
}

DEFUN (neighbor_weight,
       neighbor_weight_cmd,
       NEIGHBOR_CMD "weight <0-65535>",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Set default weight for routes from this neighbor\n"
       "default weight\n")
{
  return peer_weight_set (vty, argv[0], AFI_IP, argv[1]);
}

DEFUN (no_neighbor_weight,
       no_neighbor_weight_cmd,
       NO_NEIGHBOR_CMD "weight",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Set default weight for routes from this neighbor\n")
{
  return peer_weight_unset (vty, argv[0], AFI_IP);
}

ALIAS (no_neighbor_weight,
       no_neighbor_weight_val_cmd,
       NO_NEIGHBOR_CMD "weight <0-65535>",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Set default weight for routes from this neighbor\n"
       "default weight\n")

DEFUN (ipv6_bgp_neighbor_weight,
       ipv6_bgp_neighbor_weight_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) weight <0-65535>",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Set default weight for routes from this neighbor\n"
       "default weight\n")
{
  return peer_weight_set (vty, argv[0], AFI_IP6, argv[1]);
}

DEFUN (no_ipv6_bgp_neighbor_weight,
       no_ipv6_bgp_neighbor_weight_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) weight",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Set default weight for routes from this neighbor\n")
{
  return peer_weight_unset (vty, argv[0], AFI_IP6);
}

ALIAS (no_ipv6_bgp_neighbor_weight,
       no_ipv6_bgp_neighbor_weight_val_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) weight <0-65535>",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Set default weight for routes from this neighbor\n"
       "default weight\n")

/* neighbor soft-reconfig. */
DEFUN (neighbor_soft_reconfiguration,
       neighbor_soft_reconfiguration_cmd,
       NEIGHBOR_CMD "soft-reconfiguration inbound",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Per neighbor soft reconfiguration\n"
       "Allow inbound soft reconfiguration for this neighbor\n")
{
  int ret;
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, argv[0], bgp_node_afi (vty));
  if (! conf)
    return CMD_WARNING;

  peer = conf->peer;

  ret = peer_change_af_flag (vty, argv[0], bgp_node_afi (vty), bgp_node_safi (vty),
			     PEER_FLAG_SOFT_RECONFIG, flag_change_set);

  if (ret == CMD_SUCCESS)
    {
      if (peer->status == Established
	  && (peer->refresh_nego_old || peer->refresh_nego_new))
	bgp_route_refresh_send (peer, bgp_node_afi (vty), bgp_node_safi (vty));
      else
	BGP_EVENT_ADD (peer, BGP_Stop);
    }

  return CMD_SUCCESS;
}

DEFUN (no_neighbor_soft_reconfiguration,
       no_neighbor_soft_reconfiguration_cmd,
       NO_NEIGHBOR_CMD "soft-reconfiguration inbound",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Per neighbor soft reconfiguration\n"
       "Allow inbound soft reconfiguration for this neighbor\n")
{
  int ret;
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, argv[0], bgp_node_afi (vty));
  if (! conf)
    return CMD_WARNING;

  peer = conf->peer;

  ret = peer_change_af_flag (vty, argv[0], bgp_node_afi (vty), bgp_node_safi (vty),
			     PEER_FLAG_SOFT_RECONFIG, flag_change_unset);

  if (ret == CMD_SUCCESS)
    bgp_adj_clear (peer->adj_in[bgp_node_afi (vty)][bgp_node_safi (vty)],
		   bgp_node_safi (vty));

  return CMD_SUCCESS;
}

DEFUN (ipv6_bgp_neighbor_soft_reconfiguration,
       ipv6_bgp_neighbor_soft_reconfiguration_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) soft-reconfiguration inbound",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Per neighbor soft reconfiguration\n"
       "Allow inbound soft reconfiguration for this neighbor\n")
{
  return peer_change_af_flag (vty, argv[0], AFI_IP6, SAFI_UNICAST,
			      PEER_FLAG_SOFT_RECONFIG, flag_change_set_reset);
}

DEFUN (no_ipv6_bgp_neighbor_soft_reconfiguration,
       no_ipv6_bgp_neighbor_soft_reconfiguration_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) soft-reconfiguration inbound",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Per neighbor soft reconfiguration\n"
       "Allow inbound soft reconfiguration for this neighbor\n")
{
  int ret;
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, argv[0], AFI_IP6);
  if (! conf)
    return CMD_WARNING;

  peer = conf->peer;

  ret = peer_change_af_flag (vty, argv[0], AFI_IP6, SAFI_UNICAST,
			     PEER_FLAG_SOFT_RECONFIG, flag_change_unset);

  if (ret == CMD_SUCCESS)
    bgp_adj_clear (peer->adj_in[AFI_IP6][SAFI_UNICAST],
		   SAFI_UNICAST);

  return CMD_SUCCESS;
}


DEFUN (neighbor_route_reflector_client,
       neighbor_route_reflector_client_cmd,
       NEIGHBOR_CMD "route-reflector-client",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Configure a neighbor as Route Reflector client\n")
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, argv[0], bgp_node_afi (vty));
  if (! conf)
    return CMD_WARNING;

  peer = conf->peer;

  if (peer_sort (peer) != BGP_PEER_IBGP)
    {
      vty_out (vty, "%% Invalid command. Not an internal neighbor%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  return peer_change_af_flag (vty, argv[0], bgp_node_afi (vty), bgp_node_safi (vty),
			      PEER_FLAG_REFLECTOR_CLIENT, flag_change_set_reset);
}

DEFUN (no_neighbor_route_reflector_client,
       no_neighbor_route_reflector_client_cmd,
       NO_NEIGHBOR_CMD "route-reflector-client",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Configure a neighbor as Route Reflector client\n")
{
  return peer_change_af_flag (vty, argv[0], bgp_node_afi (vty), bgp_node_safi (vty),
			      PEER_FLAG_REFLECTOR_CLIENT, flag_change_unset_reset);
}

DEFUN (ipv6_bgp_neighbor_route_reflector_client,
       ipv6_bgp_neighbor_route_reflector_client_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) route-reflector-client",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Configure a neighbor as Route Reflector client\n")
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, argv[0], bgp_node_afi (vty));
  if (! conf)
    return CMD_WARNING;

  peer = conf->peer;

  if (peer_sort (peer) != BGP_PEER_IBGP)
    {
      vty_out (vty, "%% Invalid command. Not an internal neighbor%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  return peer_change_af_flag (vty, argv[0], AFI_IP6, SAFI_UNICAST,
			      PEER_FLAG_REFLECTOR_CLIENT, flag_change_set_reset);
}

DEFUN (no_ipv6_bgp_neighbor_route_reflector_client,
       no_ipv6_bgp_neighbor_route_reflector_client_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) route-reflector-client",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Configure a neighbor as Route Reflector client\n")
{
  return peer_change_af_flag (vty, argv[0], AFI_IP6, SAFI_UNICAST,
			      PEER_FLAG_REFLECTOR_CLIENT, flag_change_unset_reset);
}

/* neighbor route-server-client. */
DEFUN (neighbor_route_server_client,
       neighbor_route_server_client_cmd,
       NEIGHBOR_CMD "route-server-client",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Configure a neighbor as Route Server client\n")
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, argv[0], bgp_node_afi (vty)); 
  if (! conf)
    return CMD_WARNING;

  peer = conf->peer;

  if (peer_sort (peer) != BGP_PEER_EBGP)
    {
      vty_out (vty, "%% Invalid command. Not an external neighbor%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  return peer_change_af_flag (vty, argv[0], bgp_node_afi (vty), bgp_node_safi (vty),
			      PEER_FLAG_RSERVER_CLIENT, flag_change_set_reset);
}

DEFUN (no_neighbor_route_server_client,
       no_neighbor_route_server_client_cmd,
       NO_NEIGHBOR_CMD "route-server-client",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Configure a neighbor as Route Server client\n")
{
  return peer_change_af_flag (vty, argv[0], bgp_node_afi (vty), bgp_node_safi (vty),
			      PEER_FLAG_RSERVER_CLIENT, flag_change_unset_reset);
}

DEFUN (ipv6_bgp_neighbor_route_server_client,
       ipv6_bgp_neighbor_route_server_client_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) route-server-client",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Configure a neighbor as Route Server client\n")
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, argv[0], AFI_IP6); 
  if (! conf)
    return CMD_WARNING;

  peer = conf->peer;

  if (peer_sort (peer) != BGP_PEER_EBGP)
    {
      vty_out (vty, "%% Invalid command. Not an external neighbor%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }
  return peer_change_af_flag (vty, argv[0], AFI_IP6, SAFI_UNICAST,
			      PEER_FLAG_RSERVER_CLIENT, flag_change_set_reset);
}

DEFUN (no_ipv6_bgp_neighbor_route_server_client,
       no_ipv6_bgp_neighbor_route_server_client_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) route-server-client",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Configure a neighbor as Route Server client\n")
{
  return peer_change_af_flag (vty, argv[0], AFI_IP6, SAFI_UNICAST,
			      PEER_FLAG_RSERVER_CLIENT, flag_change_unset_reset);
}

/* neighbor capability route-refresh. */
DEFUN (neighbor_capability_route_refresh,
       neighbor_capability_route_refresh_cmd,
       NEIGHBOR_CMD "capability route-refresh",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Outbound capability configuration\n"
       "Advertise route-refresh capability to this neighbor\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP, 
				      PEER_FLAG_CAPABILITY_ROUTE_REFRESH, 1);
}

DEFUN (no_neighbor_capability_route_refresh,
       no_neighbor_capability_route_refresh_cmd,
       NO_NEIGHBOR_CMD "capability route-refresh",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Outbound capability configuration\n"
       "Advertise route-refresh capability to this neighbor\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP,
				      PEER_FLAG_CAPABILITY_ROUTE_REFRESH, 0);
}

DEFUN (ipv6_bgp_neighbor_capability_route_refresh,
       ipv6_bgp_neighbor_capability_route_refresh_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) capability route-refresh",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Outbound capability configuration\n"
       "Advertise route-refresh capability to this neighbor\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP6,
				      PEER_FLAG_CAPABILITY_ROUTE_REFRESH, 1);
}

DEFUN (no_ipv6_bgp_neighbor_capability_route_refresh,
       no_ipv6_bgp_neighbor_capability_route_refresh_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) capability route-refresh",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Outbound capability configuration\n"
       "Advertise route-refresh capability to this neighbor\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP6,
				      PEER_FLAG_CAPABILITY_ROUTE_REFRESH, 0);
}

/* neighbor transparent-as */
DEFUN (neighbor_transparent_as,
       neighbor_transparent_as_cmd,
       NEIGHBOR_CMD "transparent-as",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Do not append my AS number even peer is EBGP peer\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP, 
				      PEER_FLAG_TRANSPARENT_AS, 1);
}

DEFUN (no_neighbor_transparent_as,
       no_neighbor_transparent_as_cmd,
       NO_NEIGHBOR_CMD "transparent-as",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Do not append my AS number even peer is EBGP peer\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP,
				      PEER_FLAG_TRANSPARENT_AS, 0);
}

DEFUN (ipv6_bgp_neighbor_transparent_as,
       ipv6_bgp_neighbor_transparent_as_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) transparent-as",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Do not append my AS number even peer is EBGP peer\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP6,
				      PEER_FLAG_TRANSPARENT_AS, 1);
}

DEFUN (no_ipv6_bgp_neighbor_transparent_as,
       no_ipv6_bgp_neighbor_transparent_as_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) transparent-as",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Do not append my AS number even peer is EBGP peer\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP6,
				      PEER_FLAG_TRANSPARENT_AS, 0);
}

/* neighbor transparent-nexthop */
DEFUN (neighbor_transparent_nexthop,
       neighbor_transparent_nexthop_cmd,
       NEIGHBOR_CMD "transparent-nexthop",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Do not change nexthop even peer is EBGP peer\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP, 
				      PEER_FLAG_TRANSPARENT_NEXTHOP, 1);
}

DEFUN (no_neighbor_transparent_nexthop,
       no_neighbor_transparent_nexthop_cmd,
       NO_NEIGHBOR_CMD "transparent-nexthop",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Do not change nexthop even peer is EBGP peer\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP,
				      PEER_FLAG_TRANSPARENT_NEXTHOP, 0);
}

DEFUN (ipv6_bgp_neighbor_transparent_nexthop,
       ipv6_bgp_neighbor_transparent_nexthop_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) transparent-nexthop",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Do not change nexthop even peer is EBGP peer\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP6,
				      PEER_FLAG_TRANSPARENT_NEXTHOP, 1);
}

DEFUN (no_ipv6_bgp_neighbor_transparent_nexthop,
       no_ipv6_bgp_neighbor_transparent_nexthop_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) transparent-nexthop",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Do not change nexthop even peer is EBGP peer\n")
{
  return peer_change_flag_with_reset (vty, argv[0], AFI_IP6,
				      PEER_FLAG_TRANSPARENT_NEXTHOP, 0);
}

/* neighbor translate-update. */
int
peer_translate_update (struct vty *vty, char *ip_str, int afi, int safi)
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;

  peer = conf->peer;
  peer->translate_update = safi;
  return CMD_SUCCESS;
}

DEFUN (neighbor_translate_update_multicast,
       neighbor_translate_update_multicast_cmd,
       NEIGHBOR_CMD "translate-update nlri multicast",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Translate bgp updates\n"
       "Network Layer Reachable Information\n"
       "multicast information\n")
{
  return peer_translate_update (vty, argv[0], AFI_IP, SAFI_MULTICAST);
}

DEFUN (neighbor_translate_update_unimulti,
       neighbor_translate_update_unimulti_cmd,
       NEIGHBOR_CMD "translate-update nlri unicast multicast",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Translate bgp updates\n"
       "Network Layer Reachable Information\n"
       "unicast information\n"
       "multicast inforamtion\n")
{
  return peer_translate_update (vty, argv[0], AFI_IP, SAFI_UNICAST_MULTICAST);
}

DEFUN (no_neighbor_translate_update,
       no_neighbor_translate_update_cmd,
       NO_NEIGHBOR_CMD "translate-update",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Translate bgp updates\n")
{
  return peer_translate_update (vty, argv[0], AFI_IP, 0);
}

DEFUN (no_neighbor_translate_update_multicast,
       no_neighbor_translate_update_multicast_cmd,
       NO_NEIGHBOR_CMD "translate-update nlri multicast",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Translate bgp updates\n"
       "Network Layer Reachable Information\n"
       "multicast information\n")
{
  return peer_translate_update (vty, argv[0], AFI_IP, 0);
}

DEFUN (no_neighbor_translate_update_unimulti,
       no_neighbor_translate_update_unimulti_cmd,
       NO_NEIGHBOR_CMD "translate-update nlri unicast multicast",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Translate bgp updates\n"
       "Network Layer Reachable Information\n"
       "unicast information\n"
       "multicast inforamtion\n")
{
  return peer_translate_update (vty, argv[0], AFI_IP, 0);
}

/* neighbor dont-capability-negotiate */
DEFUN (neighbor_dont_capability_negotiate,
       neighbor_dont_capability_negotiate_cmd,
       NEIGHBOR_CMD "dont-capability-negotiate",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Do not perform capability negotiation\n")
{
  return peer_change_flag (vty, argv[0], AFI_IP,
			   PEER_FLAG_DONT_CAPABILITY, 1);
}

DEFUN (no_neighbor_dont_capability_negotiate,
       no_neighbor_dont_capability_negotiate_cmd,
       NO_NEIGHBOR_CMD "dont-capability-negotiate",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Do not perform capability negotiation\n")
{
  return peer_change_flag (vty, argv[0], AFI_IP,
			   PEER_FLAG_DONT_CAPABILITY, 0);
}

DEFUN (ipv6_neighbor_dont_capability_negotiate,
       ipv6_neighbor_dont_capability_negotiate_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) dont-capability-negotiate",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Do not perform capability negotiation\n")
{
  return peer_change_flag (vty, argv[0], AFI_IP6,
			   PEER_FLAG_DONT_CAPABILITY, 1);
}

DEFUN (no_ipv6_neighbor_dont_capability_negotiate,
       no_ipv6_neighbor_dont_capability_negotiate_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) dont-capability-negotiate",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Do not perform capability negotiation\n")
{
  return peer_change_flag (vty, argv[0], AFI_IP6,
			   PEER_FLAG_DONT_CAPABILITY, 0);
}

/* Override capability negotiation. */
int
peer_override_capability (struct vty *vty, char *ip_str, int afi, int set)
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  peer = conf->peer;
  
  if (set)
    {
      if (CHECK_FLAG (peer->flags, PEER_FLAG_STRICT_CAP_MATCH))
	{
	  vty_out (vty, "Can't set override-capability and strict-capability-match at the same time%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}
      SET_FLAG (peer->flags, PEER_FLAG_OVERRIDE_CAPABILITY);
    }
  else
    UNSET_FLAG (peer->flags, PEER_FLAG_OVERRIDE_CAPABILITY);
  return CMD_SUCCESS;
}

/* Override capability negotiation. */
DEFUN (neighbor_override_capability,
       neighbor_override_capability_cmd,
       NEIGHBOR_CMD "override-capability",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Override capability negotiation result\n")
{
  return peer_override_capability (vty, argv[0], AFI_IP, 1);
}

DEFUN (no_neighbor_override_capability,
       no_neighbor_override_capability_cmd,
       NO_NEIGHBOR_CMD "override-capability",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Override capability negotiation result\n")
{
  return peer_override_capability (vty, argv[0], AFI_IP, 0);
}

DEFUN (ipv6_neighbor_override_capability,
       ipv6_neighbor_override_capability_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) override-capability",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Override capability negotiation result\n")
{
  return peer_override_capability (vty, argv[0], AFI_IP6, 1);
}

DEFUN (no_ipv6_neighbor_override_capability,
       no_ipv6_neighbor_override_capability_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) override-capability",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Override capability negotiation result\n")
{
  return peer_override_capability (vty, argv[0], AFI_IP6, 0);
}

/* Strict capability match. */
int
peer_strict_capability (struct vty *vty, char *ip_str, int afi, int set)
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  peer = conf->peer;
  
  if (set)
    {
      if (CHECK_FLAG (peer->flags, PEER_FLAG_OVERRIDE_CAPABILITY))
	{
	  vty_out (vty, "Can't set override-capability and strict-capability-match at the same time%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}
      SET_FLAG (peer->flags, PEER_FLAG_STRICT_CAP_MATCH);
    }
  else
    UNSET_FLAG (peer->flags, PEER_FLAG_STRICT_CAP_MATCH);
  return CMD_SUCCESS;
}

DEFUN (neighbor_strict_capability,
       neighbor_strict_capability_cmd,
       NEIGHBOR_CMD "strict-capability-match",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Strict capability negotiation match\n")
{
  return peer_strict_capability (vty, argv[0], AFI_IP, 1);
}

DEFUN (no_neighbor_strict_capability,
       no_neighbor_strict_capability_cmd,
       NO_NEIGHBOR_CMD "strict-capability-match",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Strict capability negotiation match\n")
{
  return peer_strict_capability (vty, argv[0], AFI_IP, 0);
}

DEFUN (ipv6_neighbor_strict_capability,
       ipv6_neighbor_strict_capability_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) strict-capability-match",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Strict capability negotiation match\n")
{
  return peer_strict_capability (vty, argv[0], AFI_IP6, 1);
}

DEFUN (no_ipv6_neighbor_strict_capability,
       no_ipv6_neighbor_strict_capability_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) strict-capability-match",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Strict capability negotiation match\n")
{
  return peer_strict_capability (vty, argv[0], AFI_IP6, 0);
}

int
peer_timers_set (struct vty *vty, char *ip_str, int afi,
	         char *keep_str, char *hold_str)
{
  struct peer *peer;
  struct peer_conf *conf;
  unsigned long keepalive;
  unsigned long holdtime;
  char *endptr = NULL;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;

  peer = conf->peer;

  /* keepalive value check. */
  keepalive = strtoul (keep_str, &endptr, 10);

  if (keepalive == ULONG_MAX || *endptr != '\0')
    {
      vty_out (vty, "%% keepalive time value must be positive integer%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }
  if (keepalive > 65535)
    {
      vty_out (vty, "%% keepalive time value must be <0-65535>%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Holdtime value check. */
  holdtime = strtoul (hold_str, &endptr, 10);

  if (holdtime == ULONG_MAX || *endptr != '\0')
    {
      vty_out (vty, "%% hold time value must be positive integer%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  if (holdtime > 65535)
    {
      vty_out (vty, "%% hold time value must be <0,3-65535>%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  if (holdtime < 3 && holdtime != 0)
    {
      vty_out (vty, "%% hold time value must be either 0 or greater than 3%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Set value to the configuration. */
  peer->config |= PEER_CONFIG_TIMER;
  peer->holdtime = holdtime;
  peer->keepalive = (keepalive < holdtime / 3 ? keepalive : holdtime / 3);

  return CMD_SUCCESS;
}

int
peer_timers_unset (struct vty *vty, char *ip_str, int afi)
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;

  peer = conf->peer;

  /* Clear configuration. */
  peer->config &= ~PEER_CONFIG_TIMER;
  peer->keepalive = 0;
  peer->holdtime = 0;

  return CMD_SUCCESS;
}

DEFUN (neighbor_timers,
       neighbor_timers_cmd,
       NEIGHBOR_CMD "timers <0-65535> <0-65535>",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "BGP per neighbor timers\n"
       "Keepalive interval\n"
       "Holdtime\n")
{
  return peer_timers_set (vty, argv[0], AFI_IP, argv[1], argv[2]);
}

DEFUN (no_neighbor_timers,
       no_neighbor_timers_cmd,
       NO_NEIGHBOR_CMD "timers",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "BGP per neighbor timers\n")
{
  return peer_timers_unset (vty, argv[0], AFI_IP);
}

DEFUN (ipv6_bgp_neighbor_timers,
       ipv6_bgp_neighbor_timers_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) timers <0-65535> <0-65535>",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "BGP per neighbor timers\n"
       "Keepalive interval\n"
       "Holdtime\n")
{
  return peer_timers_set (vty, argv[0], AFI_IP6, argv[1], argv[2]);
}

DEFUN (no_ipv6_bgp_neighbor_timers,
       no_ipv6_bgp_neighbor_timers_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) timers",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "BGP per neighbor timers\n")
{
  return peer_timers_unset (vty, argv[0], AFI_IP6);
}

int
peer_timers_connect_set (struct vty *vty, char *ip_str, int afi,
			 char *time_str)
{
  struct peer *peer;
  struct peer_conf *conf;
  unsigned long connect;
  char *endptr = NULL;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;

  peer = conf->peer;

  /* Hold time value check. */
  connect = strtoul (time_str, &endptr, 10);

  if (connect == ULONG_MAX || *endptr != '\0')
    {
      vty_out (vty, "connect time value must be positive integer%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }
  if (connect > 65535)
    {
      vty_out (vty, "connect time value must be <0-65535>%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Set value to the configuration. */
  peer->config |= PEER_CONFIG_CONNECT;
  peer->connect = connect;

  /* Set value to timer setting. */
  peer->v_connect = connect;

  return CMD_SUCCESS;
}

int
peer_timers_connect_unset (struct vty *vty, char *ip_str, int afi)
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  peer = conf->peer;

  /* Clear configuration. */
  UNSET_FLAG (peer->config, PEER_CONFIG_CONNECT);
  peer->connect = 0;

  /* Set timer setting to default value. */
  peer->v_connect = BGP_DEFAULT_CONNECT_RETRY;

  return CMD_SUCCESS;
}

DEFUN (neighbor_timers_connect,
       neighbor_timers_connect_cmd,
       NEIGHBOR_CMD "timers connect <0-65535>",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "BGP per neighbor timers\n"
       "BGP connect timer\n"
       "Connect timer\n")
{
  return peer_timers_connect_set (vty, argv[0], AFI_IP, argv[1]);
}

DEFUN (no_neighbor_timers_connect,
       no_neighbor_timers_connect_cmd,
       NO_NEIGHBOR_CMD "timers connect [TIMER]",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "BGP per neighbor timers\n"
       "BGP connect timer\n"
       "Connect timer\n")
{
  return peer_timers_connect_unset (vty, argv[0], AFI_IP);
}

DEFUN (ipv6_bgp_neighbor_timers_connect,
       ipv6_bgp_neighbor_timers_connect_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) timers connect <0-65535>",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "BGP per neighbor timers\n"
       "BGP connect timer\n"
       "Connect timer\n")
{
  return peer_timers_connect_set (vty, argv[0], AFI_IP6, argv[1]);
}

DEFUN (no_ipv6_bgp_neighbor_timers_connect,
       no_ipv6_bgp_neighbor_timers_connect_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) timers connect [TIMER]",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "BGP per neighbor timers\n"
       "BGP connect timer\n"
       "Connect timer\n")
{
  return peer_timers_connect_unset (vty, argv[0], AFI_IP6);
}

int
peer_version (struct vty *vty, char *ip_str, int afi, char *str)
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  peer = conf->peer;

  /* BGP version string check. */
  if (str)
    {
      if (strcmp (str, "4") == 0)
	peer->version = BGP_VERSION_4;
      else if (strcmp (str, "4-") == 0)
	peer->version = BGP_VERSION_MP_4_DRAFT_00;
      else
	vty_out (vty, "BGP version malformed!%s", VTY_NEWLINE);
    }
  else
    peer->version = BGP_VERSION_4;

  return CMD_SUCCESS;
}

DEFUN (neighbor_version,
       neighbor_version_cmd,
       NEIGHBOR_CMD "version (4|4-)",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Neighbor's BGP version\n"
       "Border Gateway Protocol 4\n"
       "Multiprotocol Extensions for BGP-4(Old Draft)\n")
{
  return peer_version (vty, argv[0], AFI_IP, argv[1]);
}

DEFUN (no_neighbor_version,
       no_neighbor_version_cmd,
       NO_NEIGHBOR_CMD "version",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Neighbor's BGP version\n")
{
  return peer_version (vty, argv[0], AFI_IP, NULL);
}

DEFUN (ipv6_bgp_neighbor_version,
       ipv6_bgp_neighbor_version_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) version (4|4-)",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Neighbor's BGP version\n"
       "Border Gateway Protocol 4\n"
       "Multiprotocol Extensions for BGP-4(Old Draft)\n")
{
  return peer_version (vty, argv[0], AFI_IP6, argv[1]);
}

DEFUN (no_ipv6_bgp_neighbor_version,
       no_ipv6_bgp_neighbor_version_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) version",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Neighbor's BGP version\n")
{
  return peer_version (vty, argv[0], AFI_IP6, NULL);
}

/* neighbor interface */
int
peer_interface (struct vty *vty, char *ip_str, int afi, char *str)
{
  struct peer *peer;
  struct peer_conf *conf;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  peer = conf->peer;

  if (str)
    {
      if (peer->ifname)
	free (peer->ifname);
      peer->ifname = strdup (str);
    }
  else
    {
      if (peer->ifname)
	free (peer->ifname);
      peer->ifname = NULL;
    }
  return CMD_SUCCESS;
}

DEFUN (neighbor_interface,
       neighbor_interface_cmd,
       NEIGHBOR_CMD "interface WORD",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Interface\n"
       "Interface name\n")
{
  return peer_interface (vty, argv[0], AFI_IP, argv[1]);
}

DEFUN (no_neighbor_interface,
       no_neighbor_interface_cmd,
       NO_NEIGHBOR_CMD "interface WORD",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Interface\n"
       "Interface name\n")
{
  return peer_interface (vty, argv[0], AFI_IP, NULL);
}

DEFUN (ipv6_bgp_neighbor_interface,
       ipv6_bgp_neighbor_interface_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) interface WORD",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Interface\n"
       "Interface name\n")
{
  return peer_interface (vty, argv[0], AFI_IP6, argv[1]);
}

DEFUN (no_ipv6_bgp_neighbor_interface,
       no_ipv6_bgp_neighbor_interface_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) interface WORD",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Interface\n"
       "Interface name\n")
{
  return peer_interface (vty, argv[0], AFI_IP6, NULL);
}

/* Set distribute list to the peer. */
static int
bgp_distribute_set (struct vty *vty, char *ip_str, afi_t afi, safi_t safi,
		    char *name_str, char *direct_str)
{
  struct peer_conf *conf;
  struct bgp_filter *filter;
  int direct;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  if (! conf->afc[afi][safi])
    {
      vty_out (vty, "%% Activate the neighbor for the address family first%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Check filter direction. */
  if (strncmp (direct_str, "i", 1) == 0)
    direct = FILTER_IN;
  else if (strncmp (direct_str, "o", 1) == 0)
    direct = FILTER_OUT;
  else
    {
      vty_out (vty, "filter direction must be [in|out]%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  filter = &conf->filter[afi][safi];

  if (filter->dlist[direct].name)
    free (filter->dlist[direct].name);

  filter->dlist[direct].name = strdup (name_str);
  filter->dlist[direct].alist = access_list_lookup (afi, name_str);

  return CMD_SUCCESS;
}

static int
bgp_distribute_unset (struct vty *vty, char *ip_str, afi_t afi, safi_t safi,
		      char *name_str, char *direct_str)
{
  struct peer_conf *conf;
  struct bgp_filter *filter;
  int direct;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  if (! conf->afc[afi][safi])
    {
      vty_out (vty, "%% Activate the neighbor for the address family first%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }
  
  /* Check filter direction. */
  if (strncmp (direct_str, "i", 1) == 0)
    direct = FILTER_IN;
  else if (strncmp (direct_str, "o", 1) == 0)
    direct = FILTER_OUT;
  else
    {
      vty_out (vty, "distribute direction must be [in|out]%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  filter = &conf->filter[afi][safi];

  if (! filter->dlist[direct].name)
    {
      vty_out (vty, "%% There is no such filter: %s%s", name_str, VTY_NEWLINE);
      return CMD_WARNING;
    }
  if (strcmp (filter->dlist[direct].name, name_str) != 0)
    {
      vty_out (vty, "%% There is no such filter: %s%s", name_str, VTY_NEWLINE);
      return CMD_WARNING;
    }
  free (filter->dlist[direct].name);
  filter->dlist[direct].name = NULL;
  filter->dlist[direct].alist = NULL;

  return CMD_SUCCESS;
}

/* Update distribute list. */
void
bgp_distribute_update (struct access_list *access)
{
  afi_t afi;
  safi_t safi;
  int direct;
  struct listnode *nn, *nm;
  struct bgp *bgp;
  struct peer_conf *conf;
  struct bgp_filter *filter;

  LIST_LOOP (bgp_list, bgp, nn)
    {
      LIST_LOOP (bgp->peer_conf, conf, nm)
	{
	  for (afi = AFI_IP; afi < AFI_MAX; afi++)
	    for (safi = SAFI_UNICAST; safi < SAFI_MAX; safi++)
	      {
		filter = &conf->filter[afi][safi];

		for (direct = FILTER_IN; direct < FILTER_MAX; direct++)
		  {
		    if (filter->dlist[direct].name)
		      filter->dlist[direct].alist = 
			access_list_lookup (afi, filter->dlist[direct].name);
		    else
		      filter->dlist[direct].alist = NULL;
		  }
	      }
	}
    }
}

DEFUN (neighbor_distribute_list,
       neighbor_distribute_list_cmd,
       NEIGHBOR_CMD "distribute-list WORD (in|out)",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Filter updates to/from this neighbor\n"
       "IP Access-list name\n"
       "Filter incoming updates\n"
       "Filter outgoing updates\n")
{
  return bgp_distribute_set (vty, argv[0], AFI_IP, bgp_node_safi (vty),
			     argv[1], argv[2]);
}

DEFUN (no_neighbor_distribute_list,
       no_neighbor_distribute_list_cmd,
       NO_NEIGHBOR_CMD "distribute-list WORD (in|out)",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Filter updates to/from this neighbor\n"
       "IP Access-list name\n"
       "Filter incoming updates\n"
       "Filter outgoing updates\n")
{
  return bgp_distribute_unset (vty, argv[0], AFI_IP, bgp_node_safi (vty),
			       argv[1], argv[2]);
}

DEFUN (ipv6_bgp_neighbor_distribute_list,
       ipv6_bgp_neighbor_distribute_list_cmd,
       NEIGHBOR_CMD "distribute-list WORD (in|out)",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Filter updates to/from this neighbor\n"
       "IPv6 Access-list name\n"
       "Filter incoming updates\n"
       "Filter outgoing updates\n")
{
  return bgp_distribute_set (vty, argv[0], AFI_IP6, SAFI_UNICAST,
			     argv[1], argv[2]);
}

DEFUN (no_ipv6_bgp_neighbor_distribute_list,
       no_ipv6_bgp_neighbor_distribute_list_cmd,
       NO_NEIGHBOR_CMD "distribute-list WORD (in|out)",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Filter updates to/from this neighbor\n"
       "IPv6 Access-list name\n"
       "Filter incoming updates\n"
       "Filter outgoing updates\n")
{
  return bgp_distribute_unset (vty, argv[0], AFI_IP6, SAFI_UNICAST,
			       argv[1], argv[2]);
}

ALIAS (ipv6_bgp_neighbor_distribute_list,
       old_ipv6_bgp_neighbor_distribute_list_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) distribute-list WORD (in|out)",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Filter updates to/from this neighbor\n"
       "IPv6 Access-list name\n"
       "Filter incoming updates\n"
       "Filter outgoing updates\n")

ALIAS (no_ipv6_bgp_neighbor_distribute_list,
       old_no_ipv6_bgp_neighbor_distribute_list_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) distribute-list WORD (in|out)",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Filter updates to/from this neighbor\n"
       "IPv6 Access-list name\n"
       "Filter incoming updates\n"
       "Filter outgoing updates\n")

/* Set prefix list to the peer. */
static int
bgp_prefix_list_set (struct vty *vty, char *ip_str, afi_t afi, safi_t safi,
		     char *name_str, char *direct_str)
{
  struct peer_conf *conf;
  int direct;
  struct bgp_filter *filter;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  if (! conf->afc[afi][safi])
    {
      vty_out (vty, "%% Activate the neighbor for the address family first%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Check filter direction. */
  if (strncmp (direct_str, "i", 1) == 0)
    direct = FILTER_IN;
  else if (strncmp (direct_str, "o", 1) == 0)
    direct = FILTER_OUT;
  else
    {
      vty_out (vty, "vty, filter direction must be [in|out]%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  filter = &conf->filter[afi][safi];

  if (filter->plist[direct].name)
    free (filter->plist[direct].name);
  filter->plist[direct].name = strdup (name_str);
  filter->plist[direct].plist = prefix_list_lookup (afi, name_str);

  return CMD_SUCCESS;
}

static int
bgp_prefix_list_unset (struct vty *vty, char *ip_str, afi_t afi, safi_t safi,
		       char *name_str, char *direct_str)
{
  struct peer_conf *conf;
  int direct;
  struct bgp_filter *filter;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  
  /* Check filter direction. */
  if (strncmp (direct_str, "i", 1) == 0)
    direct = FILTER_IN;
  else if (strncmp (direct_str, "o", 1) == 0)
    direct = FILTER_OUT;
  else
    {
      vty_out (vty, "filter direction must be [in|out]%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  filter = &conf->filter[afi][safi];

  if (! filter->plist[direct].name)
    {
      vty_out (vty, "There is no such filter: %s%s", name_str, VTY_NEWLINE);
      return CMD_WARNING;
    }
  if (strcmp (filter->plist[direct].name, name_str) != 0)
    {
      vty_out (vty, "There is no such filter: %s%s", name_str, VTY_NEWLINE);
      return CMD_WARNING;
    }
  free (filter->plist[direct].name);
  filter->plist[direct].name = NULL;
  filter->plist[direct].plist = NULL;

  return CMD_SUCCESS;
}

/* Update prefix-list list. */
void
bgp_prefix_list_update ()
{
  struct listnode *nn, *nm;
  struct bgp *bgp;
  struct peer_conf *conf;
  struct bgp_filter *filter;
  afi_t afi;
  safi_t safi;
  int direct;

  LIST_LOOP (bgp_list, bgp, nn)
    {
      LIST_LOOP (bgp->peer_conf, conf, nm)
	{
	  for (afi = AFI_IP; afi < AFI_MAX; afi++)
	    for (safi = SAFI_UNICAST; safi < SAFI_MAX; safi++)
	      {
		filter = &conf->filter[afi][safi];

		for (direct = FILTER_IN; direct < FILTER_MAX; direct++)
		  {
		    if (filter->plist[direct].name)
		      filter->plist[direct].plist = 
			prefix_list_lookup (afi, filter->plist[direct].name);
		    else
		      filter->plist[direct].plist = NULL;
		  }
	      }
	}
    }
}

DEFUN (neighbor_prefix_list,
       neighbor_prefix_list_cmd,
       NEIGHBOR_CMD "prefix-list WORD (in|out)",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Filter updates to/from this neighbor\n"
       "Name of a prefix list\n"
       "Filter incoming updates\n"
       "Filter outgoing updates\n")
{
  return bgp_prefix_list_set (vty, argv[0], AFI_IP, bgp_node_safi (vty),
			      argv[1], argv[2]);
}

DEFUN (no_neighbor_prefix_list,
       no_neighbor_prefix_list_cmd,
       NO_NEIGHBOR_CMD "prefix-list WORD (in|out)",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Filter updates to/from this neighbor\n"
       "Name of a prefix list\n"
       "Filter incoming updates\n"
       "Filter outgoing updates\n")
{
  return bgp_prefix_list_unset (vty, argv[0], AFI_IP, bgp_node_safi (vty),
				argv[1], argv[2]);
}

DEFUN (ipv6_bgp_neighbor_prefix_list,
       ipv6_bgp_neighbor_prefix_list_cmd,
       NEIGHBOR_CMD "prefix-list WORD (in|out)",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Filter updates to/from this neighbor\n"
       "Name of a prefix list\n"
       "Filter incoming updates\n"
       "Filter outgoing updates\n")
{
  return bgp_prefix_list_set (vty, argv[0], AFI_IP6, SAFI_UNICAST,
			      argv[1], argv[2]);
}

DEFUN (no_ipv6_bgp_neighbor_prefix_list,
       no_ipv6_bgp_neighbor_prefix_list_cmd,
       NO_NEIGHBOR_CMD "prefix-list WORD (in|out)",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Filter updates to/from this neighbor\n"
       "Name of a prefix list\n"
       "Filter incoming updates\n"
       "Filter outgoing updates\n")
{
  return bgp_prefix_list_unset (vty, argv[0], AFI_IP6, SAFI_UNICAST,
				argv[1], argv[2]);
}

ALIAS (ipv6_bgp_neighbor_prefix_list,
       old_ipv6_bgp_neighbor_prefix_list_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) prefix-list WORD (in|out)",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Filter updates to/from this neighbor\n"
       "Name of a prefix list\n"
       "Filter incoming updates\n"
       "Filter outgoing updates\n")

ALIAS (no_ipv6_bgp_neighbor_prefix_list,
       old_no_ipv6_bgp_neighbor_prefix_list_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) prefix-list WORD (in|out)",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Filter updates to/from this neighbor\n"
       "Name of a prefix list\n"
       "Filter incoming updates\n"
       "Filter outgoing updates\n")

static int
bgp_aslist_set (struct vty *vty, char *ip_str, afi_t afi, safi_t safi,
		char *name_str, char *direct_str)
{
  struct as_list *as_list_lookup (char *name);
  struct peer_conf *conf;
  int direct;
  struct bgp_filter *filter;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  if (! conf->afc[afi][safi])
    {
      vty_out (vty, "%% Activate the neighbor for the address family first%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Check filter direction. */
  if (strncmp (direct_str, "i", 1) == 0)
    direct = FILTER_IN;
  else if (strncmp (direct_str, "o", 1) == 0)
    direct = FILTER_OUT;
  else
    {
      vty_out (vty, "filter direction must be [in|out]%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  filter = &conf->filter[afi][safi];

  if (filter->aslist[direct].name)
    free (filter->aslist[direct].name);

  filter->aslist[direct].name = strdup (name_str);
  filter->aslist[direct].aslist = as_list_lookup (name_str);

  return CMD_SUCCESS;
}

static int
bgp_aslist_unset (struct vty *vty, char *ip_str, afi_t afi, safi_t safi,
		  char *name_str, char *direct_str)
{
  struct peer_conf *conf;
  int direct;
  struct bgp_filter *filter;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  if (! conf->afc[afi][safi])
    {
      vty_out (vty, "%% Activate the neighbor for the address family first%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }
  
  /* Check filter direction. */
  if (strncmp (direct_str, "i", 1) == 0)
    direct = FILTER_IN;
  else if (strncmp (direct_str, "o", 1) == 0)
    direct = FILTER_OUT;
  else
    {
      vty_out (vty, "filter direction must be [in|out]%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  filter = &conf->filter[afi][safi];
  
  if (! filter->aslist[direct].name)
    {
      vty_out (vty, "There is no such filter: %s%s", name_str, VTY_NEWLINE);
      return CMD_WARNING;
    }
  if (strcmp (filter->aslist[direct].name, name_str) != 0)
    {
      vty_out (vty, "There is no such filter: %s%s", name_str, VTY_NEWLINE);
      return CMD_WARNING;
    }
  free (filter->aslist[direct].name);
  filter->aslist[direct].name = NULL;
  filter->aslist[direct].aslist = NULL;

  return CMD_SUCCESS;
}

void
bgp_aslist_update ()
{
  afi_t afi;
  safi_t safi;
  int direct;
  struct listnode *nn, *nm;
  struct bgp *bgp;
  struct peer_conf *conf;
  struct bgp_filter *filter;

  LIST_LOOP (bgp_list, bgp, nn)
    {
      LIST_LOOP (bgp->peer_conf, conf, nm)
	{
	  for (afi = AFI_IP; afi < AFI_MAX; afi++)
	    for (safi = SAFI_UNICAST; safi < SAFI_MAX; safi++)
	      {
		filter = &conf->filter[afi][safi];

		for (direct = FILTER_IN; direct < FILTER_MAX; direct++)
		  {
		    if (filter->aslist[direct].name)
		      filter->aslist[direct].aslist = 
			as_list_lookup (filter->aslist[direct].name);
		    else
		      filter->aslist[direct].aslist = NULL;
		  }
	      }
	}
    }
}

DEFUN (neighbor_filter_list,
       neighbor_filter_list_cmd,
       NEIGHBOR_CMD "filter-list WORD (in|out)",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Establish BGP filters\n"
       "AS path access-list name\n"
       "Filter incoming routes\n"
       "Filter outgoing routes\n")
{
  return bgp_aslist_set (vty, argv[0], AFI_IP, bgp_node_safi (vty),
			 argv[1], argv[2]);
}

DEFUN (no_neighbor_filter_list,
       no_neighbor_filter_list_cmd,
       NO_NEIGHBOR_CMD "filter-list WORD (in|out)",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Establish BGP filters\n"
       "AS path access-list name\n"
       "Filter incoming routes\n"
       "Filter outgoing routes\n")
{
  return bgp_aslist_unset (vty, argv[0], AFI_IP, bgp_node_safi (vty),
			   argv[1], argv[2]);
}

DEFUN (ipv6_bgp_neighbor_filter_list,
       ipv6_bgp_neighbor_filter_list_cmd,
       NEIGHBOR_CMD "filter-list WORD (in|out)",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Establish BGP filters\n"
       "AS path access-list name\n"
       "Filter incoming routes\n"
       "Filter outgoing routes\n")
{
  return bgp_aslist_set (vty, argv[0], AFI_IP6, SAFI_UNICAST, argv[1], argv[2]);
}

DEFUN (no_ipv6_bgp_neighbor_filter_list,
       no_ipv6_bgp_neighbor_filter_list_cmd,
       NO_NEIGHBOR_CMD "filter-list WORD (in|out)",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Establish BGP filters\n"
       "AS path access-list name\n"
       "Filter incoming routes\n"
       "Filter outgoing routes\n")
{
  return bgp_aslist_unset (vty, argv[0], AFI_IP6, SAFI_UNICAST, argv[1], argv[2]);
}

ALIAS (ipv6_bgp_neighbor_filter_list,
       old_ipv6_bgp_neighbor_filter_list_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) filter-list WORD (in|out)",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Establish BGP filters\n"
       "AS path access-list name\n"
       "Filter incoming routes\n"
       "Filter outgoing routes\n")

ALIAS (no_ipv6_bgp_neighbor_filter_list,
       old_no_ipv6_bgp_neighbor_filter_list_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) filter-list WORD (in|out)",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Establish BGP filters\n"
       "AS path access-list name\n"
       "Filter incoming routes\n"
       "Filter outgoing routes\n")

/* Set route-map to the peer. */
static int
bgp_route_map_set (struct vty *vty, char *ip_str, afi_t afi, safi_t safi,
		   char *name_str, char *direct_str)
{
  struct peer_conf *conf;
  int direct;
  struct bgp_filter *filter;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  if (! conf->afc[afi][safi])
    {
      vty_out (vty, "%% Activate the neighbor for the address family first%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Check filter direction. */
  if (strncmp (direct_str, "i", 1) == 0)
    direct = FILTER_IN;
  else if (strncmp (direct_str, "o", 1) == 0)
    direct = FILTER_OUT;
  else
    {
      vty_out (vty, "filter direction must be [in|out]%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  filter = &conf->filter[afi][safi];

  if (filter->map[direct].name)
    free (filter->map[direct].name);
  
  filter->map[direct].name = strdup (name_str);
  filter->map[direct].map = route_map_lookup_by_name (name_str);

  return CMD_SUCCESS;
}

/* Unset route-map from the peer. */
static int
bgp_route_map_unset (struct vty *vty, char *ip_str, afi_t afi, safi_t safi,
		     char *name_str, char *direct_str)
{
  struct peer_conf *conf;
  int direct;
  struct bgp_filter *filter;

  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  if (! conf->afc[afi][safi])
    {
      vty_out (vty, "%% Activate the neighbor for the address family first%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }
  
  /* Check filter direction. */
  if (strncmp (direct_str, "i", 1) == 0)
    direct = FILTER_IN;
  else if (strncmp (direct_str, "o", 1) == 0)
    direct = FILTER_OUT;
  else
    {
      vty_out (vty, "filter direction must be [in|out]%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  filter = &conf->filter[afi][safi];

  if (! filter->map[direct].name)
    {
      vty_out (vty, "There is no such filter: %s%s", name_str, VTY_NEWLINE);
      return CMD_WARNING;
    }
  if (strcmp (filter->map[direct].name, name_str) != 0)
    {
      vty_out (vty, "There is no such filter: %s%s", name_str, VTY_NEWLINE);
      return CMD_WARNING;
    }

  free (filter->map[direct].name);
  filter->map[direct].name = NULL;
  filter->map[direct].map = NULL;

  return CMD_SUCCESS;
}

DEFUN (neighbor_route_map,
       neighbor_route_map_cmd,
       NEIGHBOR_CMD "route-map WORD (in|out)",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Apply route map to neighbor\n"
       "Name of route map\n"
       "Apply map to incoming routes\n"
       "Apply map to outbound routes\n")
{
  return bgp_route_map_set (vty, argv[0], AFI_IP, bgp_node_safi (vty),
			    argv[1], argv[2]);
}

DEFUN (no_neighbor_route_map,
       no_neighbor_route_map_cmd,
       NO_NEIGHBOR_CMD "route-map WORD (in|out)",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Apply route map to neighbor\n"
       "Name of route map\n"
       "Apply map to incoming routes\n"
       "Apply map to outbound routes\n")
{
  return bgp_route_map_unset (vty, argv[0], AFI_IP, bgp_node_safi (vty),
			      argv[1], argv[2]);
}

DEFUN (ipv6_bgp_neighbor_route_map,
       ipv6_bgp_neighbor_route_map_cmd,
       NEIGHBOR_CMD "route-map WORD (in|out)",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Apply route map to neighbor\n"
       "Name of route map\n"
       "Apply map to incoming routes\n"
       "Apply map to outbound routes\n")
{
  return bgp_route_map_set (vty, argv[0], AFI_IP6, SAFI_UNICAST,
			    argv[1], argv[2]);
}

DEFUN (no_ipv6_bgp_neighbor_route_map,
       no_ipv6_bgp_neighbor_route_map_cmd,
       NO_NEIGHBOR_CMD "route-map WORD (in|out)",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Apply route map to neighbor\n"
       "Name of route map\n"
       "Apply map to incoming routes\n"
       "Apply map to outbound routes\n")
{
  return bgp_route_map_unset (vty, argv[0], AFI_IP6, SAFI_UNICAST,
			      argv[1], argv[2]);
}

ALIAS (ipv6_bgp_neighbor_route_map,
       old_ipv6_bgp_neighbor_route_map_cmd,
       "ipv6 bgp neighbor (A.B.C.D|X:X::X:X) route-map WORD (in|out)",
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Apply route map to neighbor\n"
       "Name of route map\n"
       "Apply map to incoming routes\n"
       "Apply map to outbound routes\n")

ALIAS (no_ipv6_bgp_neighbor_route_map,
       old_no_ipv6_bgp_neighbor_route_map_cmd,
       "no ipv6 bgp neighbor (A.B.C.D|X:X::X:X) route-map WORD (in|out)",
       NO_STR
       IPV6_STR
       BGP_STR
       NEIGHBOR_STR
       "IP address\n"
       "IPv6 address\n"
       "Apply route map to neighbor\n"
       "Name of route map\n"
       "Apply map to incoming routes\n"
       "Apply map to outbound routes\n")

int
bgp_maximum_prefix_set (struct vty *vty, char *ip_str, afi_t afi, safi_t safi,
			char *num_str, int warning)
{
  struct peer_conf *conf;
  unsigned long num;
  char *endptr = NULL;

  /* Lookup peer configuration. */
  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  if (! conf->afc[afi][safi])
    {
      vty_out (vty, "%% Activate the neighbor for the address family first%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Convert string to unsigned long. */
  num = strtoul (num_str, &endptr, 10);
  if (num == ULONG_MAX || *endptr != '\0')
    {
      vty_out (vty, "%% maximum-prefix count must be positive integer%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Set maximum prefix value. */
  conf->pmax[afi][safi] = num;
  conf->pmax_warning[afi][safi] = (warning ? 1 : 0);

  return CMD_SUCCESS;
}

int
bgp_maximum_prefix_unset (struct vty *vty, char *ip_str,
			  afi_t afi, safi_t safi)
{
  struct peer_conf *conf;

  /* Lookup peer configuration. */
  conf = peer_conf_lookup_vty (vty, ip_str, afi);
  if (! conf)
    return CMD_WARNING;
  if (! conf->afc[afi][safi])
    {
      vty_out (vty, "%% Activate the neighbor for the address family first%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  conf->pmax[afi][safi] = 0;
  conf->pmax_warning[afi][safi] =  0;

  return CMD_SUCCESS;
}

/* Maximum number of prefix configuration.  prefix count is different
   for each peer configuration.  So this configuration can be set for
   each peer configuration. */
DEFUN (neighbor_maximum_prefix,
       neighbor_maximum_prefix_cmd,
       NEIGHBOR_CMD "maximum-prefix <1-4294967295>",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Maximum number of prefix accept from this peer\n"
       "maximum no. of prefix limit\n")
{
  return bgp_maximum_prefix_set (vty, argv[0], bgp_node_afi (vty),
				 bgp_node_safi (vty), argv[1], 0);
}

DEFUN (neighbor_maximum_prefix_warning,
       neighbor_maximum_prefix_warning_cmd,
       NEIGHBOR_CMD "maximum-prefix <1-4294967295> warning-only",
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Maximum number of prefix accept from this peer\n"
       "maximum no. of prefix limit\n"
       "Only give warning message when limit is exceeded\n")
{
  return bgp_maximum_prefix_set (vty, argv[0], bgp_node_afi (vty),
				 bgp_node_safi (vty), argv[1], 1);
}

DEFUN (no_neighbor_maximum_prefix,
       no_neighbor_maximum_prefix_cmd,
       NO_NEIGHBOR_CMD "maximum-prefix",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Maximum number of prefix accept from this peer\n")
{
  return bgp_maximum_prefix_unset (vty, argv[0], bgp_node_afi (vty),
				   bgp_node_safi (vty));
}
 
ALIAS (no_neighbor_maximum_prefix,
       no_neighbor_maximum_prefix_val_cmd,
       NO_NEIGHBOR_CMD "maximum-prefix <1-4294967295>",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Maximum number of prefix accept from this peer\n"
       "maximum no. of prefix limit\n")

ALIAS (no_neighbor_maximum_prefix,
       no_neighbor_maximum_prefix_val2_cmd,
       NO_NEIGHBOR_CMD "maximum-prefix <1-4294967295> warning-only",
       NO_STR
       NEIGHBOR_STR
       NEIGHBOR_ADDR_STR
       "Maximum number of prefix accept from this peer\n"
       "maximum no. of prefix limit\n"
       "Only give warning message when limit is exceeded\n")

/* Address family configuration.  */
DEFUN (address_family_ipv4_multicast,
       address_family_ipv4_multicast_cmd,
       "address-family ipv4 multicast",
       "Enter Address Family command mode\n"
       "Address family\n"
       "Multicast\n")
{
  vty->node = BGP_IPV4M_NODE;
  return CMD_SUCCESS;
}

DEFUN (address_family_ipv6_unicast,
       address_family_ipv6_unicast_cmd,
       "address-family ipv6 unicast",
       "Enter Address Family command mode\n"
       "Address family\n"
       "unicast\n")
{
  vty->node = BGP_IPV6_NODE;
  return CMD_SUCCESS;
}

ALIAS (address_family_ipv6_unicast,
       address_family_ipv6_cmd,
       "address-family ipv6",
       "Enter Address Family command mode\n"
       "Address family\n")

DEFUN (exit_address_family,
       exit_address_family_cmd,
       "exit-address-family",
       "Exit from Address Family configuration mode\n")
{
  if (vty->node == BGP_IPV4M_NODE
      || vty->node == BGP_VPNV4_NODE
      || vty->node == BGP_IPV6_NODE)
    vty->node = BGP_NODE;
  return CMD_SUCCESS;
}

/* BGP clear types. */
enum clear_type
{
  clear_all,
  clear_peer,
  clear_peer_group,
  clear_as
};

int
peer_have_afi (struct peer *peer, int afi)
{
  return ((afi == AFI_IP && (peer->afc[AFI_IP][SAFI_UNICAST]
			     || peer->afc[AFI_IP][SAFI_MULTICAST]
			     || peer->afc[AFI_IP][SAFI_MPLS_VPN]))
	  || (afi == AFI_IP6 && (peer->afc[AFI_IP6][SAFI_UNICAST] 
				 || peer->afc[AFI_IP6][SAFI_MULTICAST]
				 || peer->afc[AFI_IP6][SAFI_MPLS_VPN])));
}

/* `clear ip bgp' functions. */
int
clear_bgp (struct vty *vty, int afi, enum clear_type type, char *arg)
{
  int cleared;
  struct peer *peer;
  struct listnode *nn;
  as_t as;
  unsigned long as_ul;
  char *endptr = NULL;
  union sockunion su;
  int ret;

  /* Clear all bgp neighbors. */
  if (type == clear_all)
    {
      LIST_LOOP (peer_list, peer, nn)
	{
	  if (peer_have_afi (peer, afi))
	    {
	      if (! CHECK_FLAG (peer->flags, PEER_FLAG_SHUTDOWN))
		{
		  UNSET_FLAG (peer->sflags, PEER_STATUS_PREFIX_OVERFLOW);
		  peer->v_start = BGP_INIT_START_TIMER;
		  BGP_EVENT_ADD (peer, BGP_Stop);
		}
	    }
	}
      vty_out (vty, "All bgp neighbors cleared%s", VTY_NEWLINE);

      return CMD_SUCCESS;
    }
  /* Clear specified peer.  Arg is string of the peer. */
  else if (type == clear_peer)
    {
      cleared = 0;

      /* Make sockunion for lookup. */
      ret = str2sockunion (arg, &su);
      if (ret < 0)
	{
	  vty_out (vty, "Malformed address: %s%s", arg, VTY_NEWLINE);
	  return CMD_WARNING;
	}

      LIST_LOOP (peer_list, peer, nn)
	{
	  if (peer_have_afi (peer, afi) && sockunion_same (&peer->su, &su))
	    {
	      if (! CHECK_FLAG (peer->flags, PEER_FLAG_SHUTDOWN))
		{
		  UNSET_FLAG (peer->sflags, PEER_STATUS_PREFIX_OVERFLOW);
		  peer->v_start = BGP_INIT_START_TIMER;
		  BGP_EVENT_ADD (peer, BGP_Stop);
		}
	      cleared = 1;
	    }
	}

      if (cleared)
	vty_out (vty, "neighbor %s is cleared%s", arg, VTY_NEWLINE);
      else
	vty_out (vty, "%%BGP: Unknown neighbor - \"%s\"%s", arg, VTY_NEWLINE);

      return CMD_SUCCESS;
    }
  /* AS based clear. */
  else if (type == clear_as)
    {
      cleared = 0;

      as_ul = strtoul(arg, &endptr, 10);

      if ((as_ul == ULONG_MAX) || (*endptr != '\0') || (as_ul > USHRT_MAX))
	{
	  vty_out (vty, "Invalid neighbor specifier: %s%s", arg, 
		   VTY_NEWLINE);
	  return CMD_SUCCESS;
	}

      as = (as_t) as_ul;

      LIST_LOOP (peer_list, peer, nn)
	{
	  if (peer_have_afi (peer, afi) && peer->as == as)
	    {
	      if (! CHECK_FLAG (peer->flags, PEER_FLAG_SHUTDOWN))
		{
		  UNSET_FLAG (peer->sflags, PEER_STATUS_PREFIX_OVERFLOW);
		  peer->v_start = BGP_INIT_START_TIMER;
		  BGP_EVENT_ADD (peer, BGP_Stop);
		}
	      cleared = 1;
	    }
	}
      if (cleared)
	vty_out (vty, "All neighbors which AS is %s cleared%s", arg, 
		 VTY_NEWLINE);
      else
	vty_out (vty, "%%BGP: No peer is configured with AS %s%s", arg,
		 VTY_NEWLINE);
           
      return CMD_SUCCESS;
    }

  /* Not reached. */
  return CMD_SUCCESS;
}

DEFUN (clear_ip_bgp_all,
       clear_ip_bgp_all_cmd,
       "clear ip bgp *",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear all peers\n")
{
  return clear_bgp (vty, AFI_IP, clear_all, NULL);
}

DEFUN (clear_ip_bgp_peer,
       clear_ip_bgp_peer_cmd, 
       "clear ip bgp (A.B.C.D|X:X::X:X)",
       CLEAR_STR
       IP_STR
       BGP_STR
       "BGP neighbor IP address to clear\n"
       "BGP neighbor IPv6 address to clear\n")
{
  return clear_bgp (vty, AFI_IP, clear_peer, argv[0]);
}

DEFUN (clear_ip_bgp_peer_group,
       clear_ip_bgp_peer_group_cmd, 
       "clear ip bgp peer-group WORD",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear all members of peer-group\n"
       "BGP peer-group name\n")
{
  return clear_bgp (vty, AFI_IP, clear_peer_group, argv[0]);
}

DEFUN (clear_ip_bgp_as,
       clear_ip_bgp_as_cmd,
       "clear ip bgp <1-65535>",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear peers with the AS number\n")
{
  return clear_bgp (vty, AFI_IP, clear_as, argv[0]);
}       

#ifdef HAVE_IPV6
DEFUN (clear_bgp_all,
       clear_bgp_all_cmd,
       "clear bgp *",
       CLEAR_STR
       BGP_STR
       "Clear all peers\n")
{
  return clear_bgp (vty, AFI_IP6, clear_all, NULL);
}

ALIAS (clear_bgp_all,
       clear_bgp_ipv6_all_cmd,
       "clear bgp ipv6 *",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "Clear all peers\n")

DEFUN (clear_bgp_peer,
       clear_bgp_peer_cmd, 
       "clear bgp (A.B.C.D|X:X::X:X)",
       CLEAR_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "BGP IPv6 neighbor to clear\n")
{
  return clear_bgp (vty, AFI_IP6, clear_peer, argv[0]);
}

ALIAS (clear_bgp_peer,
       clear_bgp_ipv6_peer_cmd, 
       "clear bgp ipv6 (A.B.C.D|X:X::X:X)",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "BGP neighbor address to clear\n"
       "BGP IPv6 neighbor to clear\n")

DEFUN (clear_bgp_peer_group,
       clear_bgp_peer_group_cmd, 
       "clear bgp peer-group WORD",
       CLEAR_STR
       BGP_STR
       "Clear all members of peer-group\n"
       "BGP peer-group name\n")
{
  return clear_bgp (vty, AFI_IP6, clear_peer_group, argv[0]);
}

ALIAS (clear_bgp_peer_group,
       clear_bgp_ipv6_peer_group_cmd, 
       "clear bgp ipv6 peer-group WORD",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "Clear all members of peer-group\n"
       "BGP peer-group name\n")

DEFUN (clear_bgp_as,
       clear_bgp_as_cmd,
       "clear bgp <1-65535>",
       CLEAR_STR
       BGP_STR
       "Clear peers with the AS number\n")
{
  return clear_bgp (vty, AFI_IP6, clear_as, argv[0]);
}

ALIAS (clear_bgp_as,
       clear_bgp_ipv6_as_cmd,
       "clear bgp ipv6 <1-65535>",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "Clear peers with the AS number\n")
#endif /* HAVE_IPV6 */

/* Clear ip bgp neighbor soft in. */
int
clear_bgp_soft_in (struct vty *vty, afi_t afi, safi_t safi, enum clear_type type, char *arg, int soft)
{
  int ret;
  union sockunion su;
  struct peer *peer;
  struct listnode *nn;
  as_t as = 0;
  unsigned long as_ul;
  char *endptr = NULL;
  int find = 0;

  if (type == clear_peer)
    {
      /* Looking up peer with IP address string. */
      ret = str2sockunion (arg, &su);
      if (ret < 0)
	{
	  vty_out (vty, "Malformed address: %s%s", arg, VTY_NEWLINE);
	  return CMD_WARNING;
	}
    }

  if (type == clear_as)
    {
      as_ul = strtoul(arg, &endptr, 10);
      if ((as_ul == ULONG_MAX) || (*endptr != '\0') || (as_ul > USHRT_MAX))
	{
	  vty_out (vty, "Invalid neighbor specifier: %s%s", arg, VTY_NEWLINE);
	  return CMD_WARNING;
	}
	as = (as_t) as_ul;
    }  

  LIST_LOOP (peer_list, peer, nn)
    {
      if ((type == clear_peer && sockunion_same (&peer->su, &su))
    || (type == clear_as && peer->as == as)
    || type == clear_all)
	{
	  find = 1;
	  if (peer->afc[afi][safi])
	    {
	      if (peer->status == Established)
		{
		  if (soft)
		    {
		      /* If neighbor has soft reconfiguration inbound flag.
			 Use Adj-RIB-In database. */
		      if (CHECK_FLAG (peer->af_flags[afi][safi], PEER_FLAG_SOFT_RECONFIG))
			{
			  if (safi == SAFI_MPLS_VPN)
			    {
			      vty_out (vty, "%%BGP: Soft-reconfiguration inbound is not supported for vpnv4%s", VTY_NEWLINE);
			      return CMD_WARNING;
			    }
			  bgp_soft_reconfig_in (peer, afi, safi);
			  vty_out (vty, "neighbor %s is inbound cleared (s)%s",
				   peer->host, VTY_NEWLINE);
			}
		      else
			{
			  /* If neighbor has route refresh capability, send route refresh
			     message to the peer. */
			  if (peer->refresh_nego_old || peer->refresh_nego_new) 
			    {
			      bgp_route_refresh_send (peer, afi, safi);
			      vty_out (vty, "neighbor %s is inbound cleared (r)%s",
					peer->host, VTY_NEWLINE);
			    }
			  else
			    vty_out (vty, "%%BGP: Inbound soft reconfig for %s not possible as it%s      has neither refresh capability, nor inbound soft reconfig%s", peer->host, VTY_NEWLINE, VTY_NEWLINE);
			}
		    }
		  else
		    {
		      /* If neighbor has route refresh capability, send route refresh
		         message to the peer. */
		      if (peer->refresh_nego_old || peer->refresh_nego_new) 
			{
			  bgp_route_refresh_send (peer, afi, safi);
			  vty_out (vty, "neighbor %s is inbound cleared (r)%s",
				   peer->host, VTY_NEWLINE);
			}
		      else
			{
			  /* If neighbor has soft reconfiguration inbound flag.
			     Use Adj-RIB-In database. */
			  if (CHECK_FLAG (peer->flags, PEER_FLAG_SOFT_RECONFIG))
			    {
			      if (safi == SAFI_MPLS_VPN)
				{
				  vty_out (vty, "%%BGP: Soft-reconfiguration inbound is not supported for vpnv4%s", VTY_NEWLINE);
				  return CMD_WARNING;
				}
			      bgp_soft_reconfig_in (peer, afi, safi);
			      vty_out (vty, "neighbor %s is inbound cleared (s)%s",
					peer->host, VTY_NEWLINE);
			    }
			  else
			    vty_out (vty, "%%BGP: Inbound soft reconfig for %s not possible as it%s      has neither refresh capability, nor inbound soft reconfig%s", peer->host, VTY_NEWLINE, VTY_NEWLINE);
			}
		    }
		}
	    }
	  else if (type == clear_peer || type == clear_as)
	    vty_out (vty, "%%BGP: Enable %s %s address family for the neighbor %s%s",
		afi == AFI_IP6 ? "IPv6" : safi == SAFI_MPLS_VPN ? "VPNv4" : "IPv4",
		safi == SAFI_MULTICAST ? "Multicast" : "Unicast",
		peer->host, VTY_NEWLINE);
	}
    }

  if (! find && type == clear_peer)
    {
      vty_out (vty, "%%BGP: Unknown neighbor - \"%s\"%s", arg, VTY_NEWLINE);
      return CMD_WARNING;
    }
  if (! find && type == clear_as)
    {
      vty_out (vty, "%%BGP: No peer is configured with AS %s%s", arg, VTY_NEWLINE);
      return CMD_WARNING;
    }

  return CMD_SUCCESS;
}

DEFUN (clear_ip_bgp_peer_soft_in,
       clear_ip_bgp_peer_soft_in_cmd,
       "clear ip bgp A.B.C.D soft in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "Soft reconfig\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_peer, argv[0], 1);
}

DEFUN (clear_ip_bgp_peer_in,
       clear_ip_bgp_peer_in_cmd,
       "clear ip bgp A.B.C.D in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_peer, argv[0], 0);
}
       
DEFUN (clear_ip_bgp_peer_ipv4_soft_in,
       clear_ip_bgp_peer_ipv4_soft_in_cmd,
       "clear ip bgp A.B.C.D ipv4 (unicast|multicast) soft in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "Address family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Soft reconfig\n"
       "Soft reconfig inbound update\n")
{
  if (strncmp (argv[1], "m", 1) == 0)
    return clear_bgp_soft_in (vty, AFI_IP, SAFI_MULTICAST, clear_peer, argv[0], 1);

  return clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_peer, argv[0], 1);
}

DEFUN (clear_ip_bgp_peer_ipv4_in,
       clear_ip_bgp_peer_ipv4_in_cmd,
       "clear ip bgp A.B.C.D ipv4 (unicast|multicast) in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "Address family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Soft reconfig inbound update\n")
{
  if (strncmp (argv[1], "m", 1) == 0)
    return clear_bgp_soft_in (vty, AFI_IP, SAFI_MULTICAST, clear_peer, argv[0], 0);

  return clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_peer, argv[0], 0);
}

DEFUN (clear_ip_bgp_as_soft_in,
       clear_ip_bgp_as_soft_in_cmd,
       "clear ip bgp <1-65535> soft in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Soft reconfig\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_as, argv[0], 1);
}

DEFUN (clear_ip_bgp_as_in,
       clear_ip_bgp_as_in_cmd,
       "clear ip bgp <1-65535> in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_as, argv[0], 0);
}

DEFUN (clear_ip_bgp_as_ipv4_soft_in,
       clear_ip_bgp_as_ipv4_soft_in_cmd,
       "clear ip bgp <1-65535> ipv4 (unicast|multicast) soft in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Address family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Soft reconfig\n"
       "Soft reconfig inbound update\n")
{
  if (strncmp (argv[1], "m", 1) == 0)
    return clear_bgp_soft_in (vty, AFI_IP, SAFI_MULTICAST, clear_as, argv[0], 1);

  return clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_as, argv[0], 1);
}

DEFUN (clear_ip_bgp_as_ipv4_in,
       clear_ip_bgp_as_ipv4_in_cmd,
       "clear ip bgp <1-65535> ipv4 (unicast|multicast) in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Address family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Soft reconfig inbound update\n")
{
  if (strncmp (argv[1], "m", 1) == 0)
    return clear_bgp_soft_in (vty, AFI_IP, SAFI_MULTICAST, clear_as, argv[0], 0);

  return clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_as, argv[0], 0);
}

DEFUN (clear_ip_bgp_all_soft_in,
       clear_ip_bgp_all_soft_in_cmd,
       "clear ip bgp * soft in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear all peers\n"
       "Soft reconfig\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_all, NULL, 1);
}

DEFUN (clear_ip_bgp_all_in,
       clear_ip_bgp_all_in_cmd,
       "clear ip bgp * in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear all peers\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_all, NULL, 0);
}

DEFUN (clear_ip_bgp_all_ipv4_soft_in,
       clear_ip_bgp_all_ipv4_soft_in_cmd,
       "clear ip bgp * ipv4 (unicast|multicast) soft in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear all peers\n"
       "Address family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Soft reconfig\n"
       "Soft reconfig inbound update\n")
{
  if (strncmp (argv[0], "m", 1) == 0)
    return clear_bgp_soft_in (vty, AFI_IP, SAFI_MULTICAST, clear_all, NULL, 1);

  return clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_all, NULL, 1);
}

DEFUN (clear_ip_bgp_all_ipv4_in,
       clear_ip_bgp_all_ipv4_in_cmd,
       "clear ip bgp * ipv4 (unicast|multicast) in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear all peers\n"
       "Address family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Soft reconfig inbound update\n")
{
  if (strncmp (argv[0], "m", 1) == 0)
    return clear_bgp_soft_in (vty, AFI_IP, SAFI_MULTICAST, clear_all, NULL, 0);

  return clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_all, NULL, 0);
}


DEFUN (clear_ip_bgp_peer_vpnv4_soft_in,
       clear_ip_bgp_peer_vpnv4_soft_in_cmd,
       "clear ip bgp A.B.C.D vpnv4 unicast soft in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "Address family\n"
       "Address Family Modifier\n"
       "Soft reconfig\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP, SAFI_MPLS_VPN, clear_peer, argv[0], 0);
}

DEFUN (clear_ip_bgp_peer_vpnv4_in,
       clear_ip_bgp_peer_vpnv4_in_cmd,
       "clear ip bgp A.B.C.D vpnv4 unicast in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "Address family\n"
       "Address Family Modifier\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP, SAFI_MPLS_VPN, clear_peer, argv[0], 0);
}

DEFUN (clear_ip_bgp_as_vpnv4_soft_in,
       clear_ip_bgp_as_vpnv4_soft_in_cmd,
       "clear ip bgp <1-65535> vpnv4 unicast soft in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Address family\n"
       "Address Family modifier\n"
       "Soft reconfig\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP, SAFI_MPLS_VPN, clear_as, argv[0], 0);
}

DEFUN (clear_ip_bgp_as_vpnv4_in,
       clear_ip_bgp_as_vpnv4_in_cmd,
       "clear ip bgp <1-65535> vpnv4 unicast in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Address family\n"
       "Address Family modifier\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP, SAFI_MPLS_VPN, clear_as, argv[0], 0);
}

DEFUN (clear_ip_bgp_all_vpnv4_soft_in,
       clear_ip_bgp_all_vpnv4_soft_in_cmd,
       "clear ip bgp * vpnv4 unicast soft in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear all peers\n"
       "Address family\n"
       "Address Family Modifier\n"
       "Soft reconfig\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP, SAFI_MPLS_VPN, clear_all, NULL, 0);
}

DEFUN (clear_ip_bgp_all_vpnv4_in,
       clear_ip_bgp_all_vpnv4_in_cmd,
       "clear ip bgp * vpnv4 unicast in",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear all peers\n"
       "Address family\n"
       "Address Family Modifier\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP, SAFI_MPLS_VPN, clear_all, NULL, 0);
}

/* Clear ip bgp neighbor soft out. */
int
clear_bgp_soft_out (struct vty *vty, afi_t afi, safi_t safi, enum clear_type type, char *arg)
{
  int ret;
  union sockunion su;
  struct peer *peer;
  struct listnode *nn;
  as_t as = 0;
  unsigned long as_ul;
  char *endptr = NULL;
  int find = 0;

  if (type == clear_peer)
    {
      /* Looking up peer with IP address string. */
      ret = str2sockunion (arg, &su);
      if (ret < 0)
	{
	  vty_out (vty, "Malformed address: %s%s", arg, VTY_NEWLINE);
	  return CMD_WARNING;
	}
    }

  if (type == clear_as)
    {
      as_ul = strtoul(arg, &endptr, 10);
      if ((as_ul == ULONG_MAX) || (*endptr != '\0') || (as_ul > USHRT_MAX))
	{
	  vty_out (vty, "Invalid neighbor specifier: %s%s", arg, VTY_NEWLINE);
	  return CMD_WARNING;
	}
      as = (as_t) as_ul;
    }

  LIST_LOOP (peer_list, peer, nn)
    {
      if ((type == clear_peer && sockunion_same (&peer->su, &su))
    || (type == clear_as && peer->as == as)
    || type == clear_all)
	{
	  find = 1;
	  if (peer->afc[afi][safi])
	    {
	      if (peer->status == Established)
		{
		  /* Performing soft out is same as receiving route
		     refresh. */
		  bgp_refresh_table (peer, afi, safi);
		  vty_out (vty, "neighbor %s is outbound cleared%s",
				peer->host, VTY_NEWLINE);
		}
	    }
	  else if (type == clear_peer || type == clear_as)
	    vty_out (vty, "%%BGP: Enable %s %s address family for the neighbor %s%s",
		afi == AFI_IP6 ? "IPv6" : safi == SAFI_MPLS_VPN ? "VPNv4" : "IPv4",
		safi == SAFI_MULTICAST ? "Multicast" : "Unicast",
		peer->host, VTY_NEWLINE);
	}
    }

  if (! find && type == clear_peer)
    {
      vty_out (vty, "%%BGP: Unknown neighbor - \"%s\"%s", arg, VTY_NEWLINE);
      return CMD_WARNING;
    }
  if (! find && type == clear_as)
    {
      vty_out (vty, "%%BGP: No peer is configured with AS %s%s", arg, VTY_NEWLINE);
      return CMD_WARNING;
    }

  return CMD_SUCCESS;
}

DEFUN (clear_ip_bgp_peer_soft_out,
       clear_ip_bgp_peer_soft_out_cmd,
       "clear ip bgp A.B.C.D soft out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "Soft reconfig\n"
       "Soft reconfig outbound update\n")
{
  return clear_bgp_soft_out (vty, AFI_IP, SAFI_UNICAST, clear_peer, argv[0]);
}

ALIAS (clear_ip_bgp_peer_soft_out,
       clear_ip_bgp_peer_out_cmd,
       "clear ip bgp A.B.C.D out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "Soft reconfig outbound update\n")

DEFUN (clear_ip_bgp_peer_ipv4_soft_out,
       clear_ip_bgp_peer_ipv4_soft_out_cmd,
       "clear ip bgp A.B.C.D ipv4 (unicast|multicast) soft out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "Address family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Soft reconfig\n"
       "Soft reconfig outbound update\n")
{
  if (strncmp (argv[1], "m", 1) == 0)
    return clear_bgp_soft_out (vty, AFI_IP, SAFI_MULTICAST, clear_peer, argv[0]);

  return clear_bgp_soft_out (vty, AFI_IP, SAFI_UNICAST, clear_peer, argv[0]);
}

ALIAS (clear_ip_bgp_peer_ipv4_soft_out,
       clear_ip_bgp_peer_ipv4_out_cmd,
       "clear ip bgp A.B.C.D ipv4 (unicast|multicast) out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "Address family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Soft reconfig outbound update\n")

DEFUN (clear_ip_bgp_as_soft_out,
       clear_ip_bgp_as_soft_out_cmd,
       "clear ip bgp <1-65535> soft out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Soft reconfig\n"
       "Soft reconfig outbound update\n")
{
  return clear_bgp_soft_out (vty, AFI_IP, SAFI_UNICAST, clear_as, argv[0]);
}

ALIAS (clear_ip_bgp_as_soft_out,
       clear_ip_bgp_as_out_cmd,
       "clear ip bgp <1-65535> out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Soft reconfig outbound update\n")

DEFUN (clear_ip_bgp_as_ipv4_soft_out,
       clear_ip_bgp_as_ipv4_soft_out_cmd,
       "clear ip bgp <1-65535> ipv4 (unicast|multicast) soft out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Address family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Soft reconfig\n"
       "Soft reconfig outbound update\n")
{
  if (strncmp (argv[1], "m", 1) == 0)
    return clear_bgp_soft_out (vty, AFI_IP, SAFI_MULTICAST, clear_as, argv[0]);

  return clear_bgp_soft_out (vty, AFI_IP, SAFI_UNICAST, clear_as, argv[0]);
}

ALIAS (clear_ip_bgp_as_ipv4_soft_out,
       clear_ip_bgp_as_ipv4_out_cmd,
       "clear ip bgp <1-65535> ipv4 (unicast|multicast) out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Address family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Soft reconfig outbound update\n")

DEFUN (clear_ip_bgp_all_soft_out,
       clear_ip_bgp_all_soft_out_cmd,
       "clear ip bgp * soft out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear all peers\n"
       "Soft reconfig\n"
       "Soft reconfig outbound update\n")
{
  return clear_bgp_soft_out (vty, AFI_IP, SAFI_UNICAST, clear_all, NULL);
}

ALIAS (clear_ip_bgp_all_soft_out,
       clear_ip_bgp_all_out_cmd,
       "clear ip bgp * out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear all peers\n"
       "Soft reconfig outbound update\n")

DEFUN (clear_ip_bgp_all_ipv4_soft_out,
       clear_ip_bgp_all_ipv4_soft_out_cmd,
       "clear ip bgp * ipv4 (unicast|multicast) soft out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear all peers\n"
       "Address family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Soft reconfig\n"
       "Soft reconfig outbound update\n")
{
  if (strncmp (argv[0], "m", 1) == 0)
    return clear_bgp_soft_out (vty, AFI_IP, SAFI_MULTICAST, clear_all, NULL);

  return clear_bgp_soft_out (vty, AFI_IP, SAFI_UNICAST, clear_all, NULL);
}

ALIAS (clear_ip_bgp_all_ipv4_soft_out,
       clear_ip_bgp_all_ipv4_out_cmd,
       "clear ip bgp * ipv4 (unicast|multicast) out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear all peers\n"
       "Address family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Soft reconfig outbound update\n")

DEFUN (clear_ip_bgp_peer_vpnv4_soft_out,
       clear_ip_bgp_peer_vpnv4_soft_out_cmd,
       "clear ip bgp A.B.C.D vpnv4 unicast soft out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "Address family\n"
       "Address Family Modifier\n"
       "Soft reconfig\n"
       "Soft reconfig outbound update\n")
{
  return clear_bgp_soft_out (vty, AFI_IP, SAFI_MPLS_VPN, clear_peer, argv[0]);
}

ALIAS (clear_ip_bgp_peer_vpnv4_soft_out,
       clear_ip_bgp_peer_vpnv4_out_cmd,
       "clear ip bgp A.B.C.D vpnv4 unicast out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "Address family\n"
       "Address Family Modifier\n"
       "Soft reconfig outbound update\n")

DEFUN (clear_ip_bgp_as_vpnv4_soft_out,
       clear_ip_bgp_as_vpnv4_soft_out_cmd,
       "clear ip bgp <1-65535> vpnv4 unicast soft out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Address family\n"
       "Address Family modifier\n"
       "Soft reconfig\n"
       "Soft reconfig outbound update\n")
{
  return clear_bgp_soft_out (vty, AFI_IP, SAFI_MPLS_VPN, clear_as, argv[0]);
}

ALIAS (clear_ip_bgp_as_vpnv4_soft_out,
       clear_ip_bgp_as_vpnv4_out_cmd,
       "clear ip bgp <1-65535> vpnv4 unicast out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Address family\n"
       "Address Family modifier\n"
       "Soft reconfig outbound update\n")

DEFUN (clear_ip_bgp_all_vpnv4_soft_out,
       clear_ip_bgp_all_vpnv4_soft_out_cmd,
       "clear ip bgp * vpnv4 unicast soft out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear all peers\n"
       "Address family\n"
       "Address Family Modifier\n"
       "Soft reconfig\n"
       "Soft reconfig outbound update\n")
{
  return clear_bgp_soft_out (vty, AFI_IP, SAFI_MPLS_VPN, clear_all, NULL);
}

ALIAS (clear_ip_bgp_all_vpnv4_soft_out,
       clear_ip_bgp_all_vpnv4_out_cmd,
       "clear ip bgp * vpnv4 unicast out",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear all peers\n"
       "Address family\n"
       "Address Family Modifier\n"
       "Soft reconfig outbound update\n")

/* soft reset both inbound and outbound */
DEFUN (clear_ip_bgp_peer_soft,
       clear_ip_bgp_peer_soft_cmd,
       "clear ip bgp A.B.C.D soft",
       CLEAR_STR
       IP_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "Soft reconfig\n")
{
  if (clear_bgp_soft_out (vty, AFI_IP, SAFI_UNICAST, clear_peer, argv[0]) == CMD_SUCCESS)
    clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_peer, argv[0], 0);

  return CMD_SUCCESS;
}

DEFUN (clear_ip_bgp_peer_ipv4_soft,
       clear_ip_bgp_peer_ipv4_soft_cmd,
       "clear ip bgp A.B.C.D ipv4 (unicast|multicast) soft",
       CLEAR_STR
       IP_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "Address family\n"
       "Address Family Modifier\n"
       "Address Family Modifier\n"
       "Soft reconfig\n")
{
  if (strncmp (argv[1], "m", 1) == 0)
    {
      if (clear_bgp_soft_out (vty, AFI_IP, SAFI_MULTICAST, clear_peer, argv[0]) == CMD_SUCCESS)
	clear_bgp_soft_in (vty, AFI_IP, SAFI_MULTICAST, clear_peer, argv[0], 0);
    }
  else
    if (clear_bgp_soft_out (vty, AFI_IP, SAFI_UNICAST, clear_peer, argv[0]) == CMD_SUCCESS)
      clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_peer, argv[0], 0);

  return CMD_SUCCESS;
}

DEFUN (clear_ip_bgp_as_soft,
       clear_ip_bgp_as_soft_cmd,
       "clear ip bgp <1-65535> soft",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Soft reconfig\n")
{
  if (clear_bgp_soft_out (vty, AFI_IP, SAFI_UNICAST, clear_as, argv[0]) == CMD_SUCCESS)
    clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_as, argv[0], 0);

  return CMD_SUCCESS;
}

DEFUN (clear_ip_bgp_as_ipv4_soft,
       clear_ip_bgp_as_ipv4_soft_cmd,
       "clear ip bgp <1-65535> ipv4 (unicast|multicast) soft",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Address family\n"
       "Address Family Modifier\n"
       "Address Family Modifier\n"
       "Soft reconfig\n")
{
  if (strncmp (argv[1], "m", 1) == 0)
    {
      if (clear_bgp_soft_out (vty, AFI_IP, SAFI_MULTICAST, clear_as, argv[0]) == CMD_SUCCESS)
	clear_bgp_soft_in (vty, AFI_IP, SAFI_MULTICAST, clear_as, argv[0], 0);
    }
  else
    if (clear_bgp_soft_out (vty, AFI_IP, SAFI_UNICAST, clear_as, argv[0]) == CMD_SUCCESS)
      clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_as, argv[0], 0);
  
  return CMD_SUCCESS;
}

DEFUN (clear_ip_bgp_all_soft,
       clear_ip_bgp_all_soft_cmd,
       "clear ip bgp * soft",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear all peers\n"
       "Soft reconfig\n")
{
  if (clear_bgp_soft_out (vty, AFI_IP, SAFI_UNICAST, clear_all, NULL) == CMD_SUCCESS)
    clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_all, NULL, 0);

  return CMD_SUCCESS;
}

DEFUN (clear_ip_bgp_all_ipv4_soft,
       clear_ip_bgp_all_ipv4_soft_cmd,
       "clear ip bgp * ipv4 (unicast|multicast) soft",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear all peers\n"
       "Address family\n"
       "Address Family Modifier\n"
       "Address Family Modifier\n"
       "Soft reconfig\n")
{
  if (strncmp (argv[0], "m", 1) == 0)
    {
      if (clear_bgp_soft_out (vty, AFI_IP, SAFI_MULTICAST, clear_all, NULL) == CMD_SUCCESS)
	clear_bgp_soft_in (vty, AFI_IP, SAFI_MULTICAST, clear_all, NULL, 0);
    }
  else
    if (clear_bgp_soft_out (vty, AFI_IP, SAFI_UNICAST, clear_all, NULL) == CMD_SUCCESS)
      clear_bgp_soft_in (vty, AFI_IP, SAFI_UNICAST, clear_all, NULL, 0);

  return CMD_SUCCESS;
}

DEFUN (clear_ip_bgp_peer_vpnv4_soft,
       clear_ip_bgp_peer_vpnv4_soft_cmd,
       "clear ip bgp A.B.C.D vpnv4 unicast soft",
       CLEAR_STR
       IP_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "Address family\n"
       "Address Family Modifier\n"
       "Soft reconfig\n")
{
  if (clear_bgp_soft_out (vty, AFI_IP, SAFI_MPLS_VPN, clear_peer, argv[0]) == CMD_SUCCESS)
    clear_bgp_soft_in (vty, AFI_IP, SAFI_MPLS_VPN, clear_peer, argv[0], 0);

  return CMD_SUCCESS;
}

DEFUN (clear_ip_bgp_as_vpnv4_soft,
       clear_ip_bgp_as_vpnv4_soft_cmd,
       "clear ip bgp <1-65535> vpnv4 unicast soft",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Address family\n"
       "Address Family Modifier\n"
       "Soft reconfig\n")
{
  if (clear_bgp_soft_out (vty, AFI_IP, SAFI_MPLS_VPN, clear_as, argv[0]) == CMD_SUCCESS)
    clear_bgp_soft_in (vty, AFI_IP, SAFI_MPLS_VPN, clear_as, argv[0], 0);

  return CMD_SUCCESS;
}

DEFUN (clear_ip_bgp_all_vpnv4_soft,
       clear_ip_bgp_all_vpnv4_soft_cmd,
       "clear ip bgp * vpnv4 unicast soft",
       CLEAR_STR
       IP_STR
       BGP_STR
       "Clear all peers\n"
       "Address family\n"
       "Address Family Modifier\n"
       "Soft reconfig\n")
{
  if (clear_bgp_soft_out (vty, AFI_IP, SAFI_MPLS_VPN, clear_all, NULL) == CMD_SUCCESS)
    clear_bgp_soft_in (vty, AFI_IP, SAFI_MPLS_VPN, clear_all, NULL, 0);

  return CMD_SUCCESS;
}
#ifdef HAVE_IPV6
DEFUN (clear_bgp_peer_soft_in,
       clear_bgp_peer_soft_in_cmd,
       "clear bgp (A.B.C.D|X:X::X:X) soft in",
       CLEAR_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "BGP IPv6 neighbor to clear\n"
       "Soft reconfig\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP6, SAFI_UNICAST, clear_peer, argv[0], 1);
}

ALIAS (clear_bgp_peer_soft_in,
       clear_bgp_ipv6_peer_soft_in_cmd,
       "clear bgp ipv6 (A.B.C.D|X:X::X:X) soft in",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "BGP neighbor address to clear\n"
       "BGP IPv6 neighbor to clear\n"
       "Soft reconfig\n"
       "Soft reconfig inbound update\n")

DEFUN (clear_bgp_peer_in,
       clear_bgp_peer_in_cmd,
       "clear bgp (A.B.C.D|X:X::X:X) in",
       CLEAR_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "BGP IPv6 neighbor to clear\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP6, SAFI_UNICAST, clear_peer, argv[0], 0);
}

ALIAS (clear_bgp_peer_in,
       clear_bgp_ipv6_peer_in_cmd,
       "clear bgp ipv6 (A.B.C.D|X:X::X:X) in",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "BGP neighbor address to clear\n"
       "BGP IPv6 neighbor to clear\n"
       "Soft reconfig inbound update\n")

DEFUN (clear_bgp_as_soft_in,
       clear_bgp_as_soft_in_cmd,
       "clear bgp <1-65535> soft in",
       CLEAR_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Soft reconfig\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP6, SAFI_UNICAST, clear_as, argv[0], 1);
}

ALIAS (clear_bgp_as_soft_in,
       clear_bgp_ipv6_as_soft_in_cmd,
       "clear bgp ipv6 <1-65535> soft in",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "Clear peers with the AS number\n"
       "Soft reconfig\n"
       "Soft reconfig inbound update\n")

DEFUN (clear_bgp_as_in,
       clear_bgp_as_in_cmd,
       "clear bgp <1-65535> in",
       CLEAR_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP6, SAFI_UNICAST, clear_as, argv[0], 0);
}

ALIAS (clear_bgp_as_in,
       clear_bgp_ipv6_as_in_cmd,
       "clear bgp ipv6 <1-65535> in",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "Clear peers with the AS number\n"
       "Soft reconfig inbound update\n")

DEFUN (clear_bgp_all_soft_in,
       clear_bgp_all_soft_in_cmd,
       "clear bgp * soft in",
       CLEAR_STR
       BGP_STR
       "Clear all peers\n"
       "Soft reconfig\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP6, SAFI_UNICAST, clear_all, NULL, 1);
}

ALIAS (clear_bgp_all_soft_in,
       clear_bgp_ipv6_all_soft_in_cmd,
       "clear bgp ipv6 * soft in",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "Clear all peers\n"
       "Soft reconfig\n"
       "Soft reconfig inbound update\n")

DEFUN (clear_bgp_all_in,
       clear_bgp_all_in_cmd,
       "clear bgp * in",
       CLEAR_STR
       BGP_STR
       "Clear all peers\n"
       "Soft reconfig inbound update\n")
{
  return clear_bgp_soft_in (vty, AFI_IP6, SAFI_UNICAST, clear_all, NULL, 0);
}

ALIAS (clear_bgp_all_in,
       clear_bgp_ipv6_all_in_cmd,
       "clear bgp ipv6 * in",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "Clear all peers\n"
       "Soft reconfig inbound update\n")

DEFUN (clear_bgp_peer_soft_out,
       clear_bgp_peer_soft_out_cmd,
       "clear bgp (A.B.C.D|X:X::X:X) soft out",
       CLEAR_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "BGP IPv6 neighbor to clear\n"
       "Soft reconfig\n"
       "Soft reconfig outbound update\n")
{
  return clear_bgp_soft_out (vty, AFI_IP6, SAFI_UNICAST, clear_peer, argv[0]);
}

ALIAS (clear_bgp_peer_soft_out,
       clear_bgp_ipv6_peer_soft_out_cmd,
       "clear bgp ipv6 (A.B.C.D|X:X::X:X) soft out",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "BGP neighbor address to clear\n"
       "BGP IPv6 neighbor to clear\n"
       "Soft reconfig\n"
       "Soft reconfig outbound update\n")

ALIAS (clear_bgp_peer_soft_out,
       clear_bgp_peer_out_cmd,
       "clear bgp (A.B.C.D|X:X::X:X) out",
       CLEAR_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "BGP IPv6 neighbor to clear\n"
       "Soft reconfig outbound update\n")

ALIAS (clear_bgp_peer_soft_out,
       clear_bgp_ipv6_peer_out_cmd,
       "clear bgp ipv6 (A.B.C.D|X:X::X:X) out",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "BGP neighbor address to clear\n"
       "BGP IPv6 neighbor to clear\n"
       "Soft reconfig outbound update\n")

DEFUN (clear_bgp_as_soft_out,
       clear_bgp_as_soft_out_cmd,
       "clear bgp <1-65535> soft out",
       CLEAR_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Soft reconfig\n"
       "Soft reconfig outbound update\n")
{
  return clear_bgp_soft_out (vty, AFI_IP6, SAFI_UNICAST, clear_as, argv[0]);
}

ALIAS (clear_bgp_as_soft_out,
       clear_bgp_ipv6_as_soft_out_cmd,
       "clear bgp ipv6 <1-65535> soft out",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "Clear peers with the AS number\n"
       "Soft reconfig\n"
       "Soft reconfig outbound update\n")

ALIAS (clear_bgp_as_soft_out,
       clear_bgp_as_out_cmd,
       "clear bgp <1-65535> out",
       CLEAR_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Soft reconfig outbound update\n")

ALIAS (clear_bgp_as_soft_out,
       clear_bgp_ipv6_as_out_cmd,
       "clear bgp ipv6 <1-65535> out",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "Clear peers with the AS number\n"
       "Soft reconfig outbound update\n")

DEFUN (clear_bgp_all_soft_out,
       clear_bgp_all_soft_out_cmd,
       "clear bgp * soft out",
       CLEAR_STR
       BGP_STR
       "Clear all peers\n"
       "Soft reconfig\n"
       "Soft reconfig outbound update\n")
{
  return clear_bgp_soft_out (vty, AFI_IP6, SAFI_UNICAST, clear_all, NULL);
}

ALIAS (clear_bgp_all_soft_out,
       clear_bgp_ipv6_all_soft_out_cmd,
       "clear bgp ipv6 * soft out",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "Clear all peers\n"
       "Soft reconfig\n"
       "Soft reconfig outbound update\n")

ALIAS (clear_bgp_all_soft_out,
       clear_bgp_all_out_cmd,
       "clear bgp * out",
       CLEAR_STR
       BGP_STR
       "Clear all peers\n"
       "Soft reconfig outbound update\n")

ALIAS (clear_bgp_all_soft_out,
       clear_bgp_ipv6_all_out_cmd,
       "clear bgp ipv6 * out",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "Clear all peers\n"
       "Soft reconfig outbound update\n")

DEFUN (clear_bgp_peer_soft,
       clear_bgp_peer_soft_cmd,
       "clear bgp (A.B.C.D|X:X::X:X) soft",
       CLEAR_STR
       BGP_STR
       "BGP neighbor address to clear\n"
       "BGP IPv6 neighbor to clear\n"
       "Soft reconfig\n")
{
  if (clear_bgp_soft_out (vty, AFI_IP6, SAFI_UNICAST, clear_peer, argv[0]) == CMD_SUCCESS)
    clear_bgp_soft_in (vty, AFI_IP6, SAFI_UNICAST, clear_peer, argv[0], 0);

  return CMD_SUCCESS;
}

ALIAS (clear_bgp_peer_soft,
       clear_bgp_ipv6_peer_soft_cmd,
       "clear bgp ipv6 (A.B.C.D|X:X::X:X) soft",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "BGP neighbor address to clear\n"
       "BGP IPv6 neighbor to clear\n"
       "Soft reconfig\n")

DEFUN (clear_bgp_as_soft,
       clear_bgp_as_soft_cmd,
       "clear bgp <1-65535> soft",
       CLEAR_STR
       BGP_STR
       "Clear peers with the AS number\n"
       "Soft reconfig\n")
{
  if (clear_bgp_soft_out (vty, AFI_IP6, SAFI_UNICAST, clear_as, argv[0]) == CMD_SUCCESS)
    clear_bgp_soft_in (vty, AFI_IP6, SAFI_UNICAST, clear_as, argv[0], 0);

  return CMD_SUCCESS;
}

ALIAS (clear_bgp_as_soft,
       clear_bgp_ipv6_as_soft_cmd,
       "clear bgp ipv6 <1-65535> soft",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "Clear peers with the AS number\n"
       "Soft reconfig\n")

DEFUN (clear_bgp_all_soft,
       clear_bgp_all_soft_cmd,
       "clear bgp * soft",
       CLEAR_STR
       BGP_STR
       "Clear all peers\n"
       "Soft reconfig\n")
{
  if (clear_bgp_soft_out (vty, AFI_IP6, SAFI_UNICAST, clear_all, NULL) == CMD_SUCCESS)
    clear_bgp_soft_in (vty, AFI_IP6, SAFI_UNICAST, clear_all, NULL, 0);

  return CMD_SUCCESS;
}

ALIAS (clear_bgp_all_soft,
       clear_bgp_ipv6_all_soft_cmd,
       "clear bgp ipv6 * soft",
       CLEAR_STR
       BGP_STR
       "Address family\n"
       "Clear all peers\n"
       "Soft reconfig\n")
#endif /* HAVE_IPV6 */

/* Show BGP peer's summary information. */
int
bgp_show_summary (struct vty *vty, int afi, int safi)
{
  struct bgp *bgp;
  struct peer *peer;
  struct peer_conf *conf;
  struct listnode *nn;
  struct listnode *nm;
  int count = 0;
  char timebuf[BGP_UPTIME_LEN];
  int len;

  /* Header string for each address family. */
  static char header[] = "Neighbor        V    AS MsgRcvd MsgSent   TblVer  InQ OutQ Up/Down  State/PfxRcd";

  LIST_LOOP (bgp_list, bgp, nn)
    {
      LIST_LOOP (bgp->peer_conf, conf, nm)
	{
	  peer = conf->peer;

	  if (conf->afc[afi][safi])
	    {
	      if (! count)
		{
		  vty_out (vty,
			   "BGP router identifier %s, local AS number %d%s",
                           inet_ntoa (bgp->id), bgp->as, VTY_NEWLINE);
		  vty_out (vty, 
			   "%ld BGP AS-PATH entries%s", aspath_count (),
			   VTY_NEWLINE);
		  vty_out (vty, 
			   "%ld BGP community entries%s", community_count (),
			   VTY_NEWLINE);

		  if (CHECK_FLAG(bgp->config, BGP_CONFIG_DAMPENING))
		    vty_out (vty, "Dampening enabled.%s", VTY_NEWLINE);
		  vty_out (vty, "%s", VTY_NEWLINE);
		  vty_out (vty, "%s%s", header, VTY_NEWLINE);
		}
	      count++;

	      len = vty_out (vty, "%s", peer->host);
	      len = 16 - len;
	      if (len < 1)
		vty_out (vty, "%s%*s", VTY_NEWLINE, 16, " ");
	      else
		vty_out (vty, "%*s", len, " ");

	      switch (peer->version) 
		{
		  case BGP_VERSION_4:
		    vty_out (vty, "4 ");
		    break;
		  case BGP_VERSION_MP_4_DRAFT_00:
		    vty_out (vty, "4-");
		    break;
		}

	      vty_out (vty, "%5d %7d %7d %8d %4d %4ld ",
		       peer->as,
		       peer->open_in + peer->update_in +
		       peer->keepalive_in + peer->notify_in + peer->refresh_in,
		       peer->open_out + peer->update_out +
		       peer->keepalive_out + peer->notify_out + peer->refresh_out,
		       0, 0, peer->obuf->count);

	      vty_out (vty, "%8s", 
		       peer_uptime (peer->uptime, timebuf, BGP_UPTIME_LEN));

	      if (peer->status == Established)
		{
		  vty_out (vty, " %8ld", conf->pcount[afi][safi]);
		}
	      else
		{
		  if (CHECK_FLAG (peer->flags, PEER_FLAG_SHUTDOWN))
		    vty_out (vty, " Idle (Admin)");
		  else if (CHECK_FLAG (peer->sflags, PEER_STATUS_PREFIX_OVERFLOW))
		    vty_out (vty, " Idle (PfxCt)");
		  else
		    vty_out (vty, " %-11s", LOOKUP(bgp_status_msg, peer->status));
		}

	      vty_out (vty, "%s", VTY_NEWLINE);
	    }
	}
    }

  if (count)
    vty_out (vty, "%sTotal number of neighbors %d%s", VTY_NEWLINE,
	     count, VTY_NEWLINE);
  else
    vty_out (vty, "No %s neighbor is configured%s",
	     afi == AFI_IP ? "IPv4" : "IPv6", VTY_NEWLINE);
  return CMD_SUCCESS;
}

/* `show ip bgp summary' commands. */
DEFUN (show_ip_bgp_summary, 
       show_ip_bgp_summary_cmd,
       "show ip bgp summary",
       SHOW_STR
       IP_STR
       BGP_STR
       "Summary of BGP neighbor status\n")
{
  return bgp_show_summary (vty, AFI_IP, SAFI_UNICAST);
}

DEFUN (show_ip_bgp_ipv4_summary, 
       show_ip_bgp_ipv4_summary_cmd,
       "show ip bgp ipv4 (unicast|multicast) summary",
       SHOW_STR
       IP_STR
       BGP_STR
       "Address family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Summary of BGP neighbor status\n")
{
  if (strncmp (argv[0], "m", 1) == 0)
    return bgp_show_summary (vty, AFI_IP, SAFI_MULTICAST);

  return bgp_show_summary (vty, AFI_IP, SAFI_UNICAST);
}

DEFUN (show_ip_bgp_vpnv4_all_summary,
       show_ip_bgp_vpnv4_all_summary_cmd,
       "show ip bgp vpnv4 all summary",
       SHOW_STR
       IP_STR
       BGP_STR
       "Display VPNv4 NLRI specific information\n"
       "Display information about all VPNv4 NLRIs\n"
       "Summary of BGP neighbor status\n")
{
  return bgp_show_summary (vty, AFI_IP, SAFI_MPLS_VPN);
}

DEFUN (show_ip_bgp_vpnv4_rd_summary,
       show_ip_bgp_vpnv4_rd_summary_cmd,
       "show ip bgp vpnv4 rd ASN:nn_or_IP-address:nn summary",
       SHOW_STR
       IP_STR
       BGP_STR
       "Display VPNv4 NLRI specific information\n"
       "Display information for a route distinguisher\n"
       "VPN Route Distinguisher\n"
       "Summary of BGP neighbor status\n")
{
  int ret;
  struct prefix_rd prd;

  ret = str2prefix_rd (argv[0], &prd);
  if (! ret)
    {
      vty_out (vty, "%% Malformed Route Distinguisher%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  return bgp_show_summary (vty, AFI_IP, SAFI_MPLS_VPN);
}

#ifdef HAVE_IPV6
DEFUN (show_bgp_summary, 
       show_bgp_summary_cmd,
       "show bgp summary",
       SHOW_STR
       BGP_STR
       "Summary of BGP neighbor status\n")
{
  return bgp_show_summary (vty, AFI_IP6, SAFI_UNICAST);
}

ALIAS (show_bgp_summary, 
       show_bgp_ipv6_summary_cmd,
       "show bgp ipv6 summary",
       SHOW_STR
       BGP_STR
       "Address family\n"
       "Summary of BGP neighbor status\n")

/* old command */
DEFUN (show_ipv6_bgp_summary, 
       show_ipv6_bgp_summary_cmd,
       "show ipv6 bgp summary",
       SHOW_STR
       IPV6_STR
       BGP_STR
       "Summary of BGP neighbor status\n")
{
  return bgp_show_summary (vty, AFI_IP6, SAFI_UNICAST);
}

/* old command */
DEFUN (show_ipv6_mbgp_summary, 
       show_ipv6_mbgp_summary_cmd,
       "show ipv6 mbgp summary",
       SHOW_STR
       IPV6_STR
       MBGP_STR
       "Summary of BGP neighbor status\n")
{
  return bgp_show_summary (vty, AFI_IP6, SAFI_MULTICAST);
}
#endif /* HAVE_IPV6 */

/* Show BGP peer's information. */
enum show_type
{
  show_all,
  show_peer
};

/* Return next event time. */
int
bgp_next_timer (struct thread *thread)
{
  struct timeval timer_now;
  gettimeofday (&timer_now, NULL);
  return thread->u.sands.tv_sec - timer_now.tv_sec;
}

void
bgp_show_peer_afi (struct vty *vty, struct peer_conf *conf, afi_t afi, safi_t safi)
{
  struct bgp_filter *filter;
  struct peer *p;

  p = conf->peer;
  filter = &conf->filter[afi][safi];

  vty_out (vty, " For address family: %s %s%s",
	   afi == AFI_IP6 ? "IPv6" :
	   safi == SAFI_MPLS_VPN ? "VPNv4" : "IPv4",
	   safi == SAFI_MULTICAST ? "Multicast" : "Unicast",
	   VTY_NEWLINE);
  if (CHECK_FLAG (p->af_flags[afi][safi], PEER_FLAG_REFLECTOR_CLIENT))
    vty_out (vty, "  Route-Reflector Client%s", VTY_NEWLINE);
  if (CHECK_FLAG (p->af_flags[afi][safi], PEER_FLAG_RSERVER_CLIENT))
    vty_out (vty, "  Route-Server Client%s", VTY_NEWLINE);
  if (CHECK_FLAG (p->af_flags[afi][safi], PEER_FLAG_SOFT_RECONFIG))
    vty_out (vty, "  Inbound soft reconfiguration allowed%s", VTY_NEWLINE);
  if (CHECK_FLAG (p->af_flags[afi][safi], PEER_FLAG_NEXTHOP_SELF))
    vty_out (vty, "  NEXT_HOP is always this router%s", VTY_NEWLINE);
  if (CHECK_FLAG (p->af_flags[afi][safi], PEER_FLAG_SEND_COMMUNITY)
      || CHECK_FLAG (p->af_flags[afi][safi], PEER_FLAG_SEND_EXT_COMMUNITY))
    {
      vty_out (vty, "  Community attribute sent to this neighbor");
      if (CHECK_FLAG (p->af_flags[afi][safi], PEER_FLAG_SEND_COMMUNITY)
	&& CHECK_FLAG (p->af_flags[afi][safi], PEER_FLAG_SEND_EXT_COMMUNITY))
	vty_out (vty, " (both)%s", VTY_NEWLINE);
      else if (CHECK_FLAG (p->af_flags[afi][safi], PEER_FLAG_SEND_EXT_COMMUNITY))
	vty_out (vty, " (extended)%s", VTY_NEWLINE);
      else 
	vty_out (vty, " (standard)%s", VTY_NEWLINE);
    }
  if (filter->plist[FILTER_IN].name ||
      filter->dlist[FILTER_IN].name ||
      filter->aslist[FILTER_IN].name ||
      filter->map[FILTER_IN].name)
    vty_out (vty, "  Inbound path policy configured%s", VTY_NEWLINE);
  if (filter->plist[FILTER_OUT].name ||
      filter->dlist[FILTER_OUT].name ||
      filter->aslist[FILTER_OUT].name ||
      filter->map[FILTER_OUT].name)
    vty_out (vty, "  Outbound path policy configured%s", VTY_NEWLINE);

  /* prefix-list */
  if (filter->plist[FILTER_IN].name)
    vty_out (vty, "  Incoming update prefix filter list is %s%s%s",
	     filter->plist[FILTER_IN].plist ? "*" : "",
	     filter->plist[FILTER_IN].name,
	     VTY_NEWLINE);
  if (filter->plist[FILTER_OUT].name)
    vty_out (vty, "  Outgoing update prefix filter list is %s%s%s",
	     filter->plist[FILTER_OUT].plist ? "*" : "",
	     filter->plist[FILTER_OUT].name,
	     VTY_NEWLINE);

  /* distribute-list */
  if (filter->dlist[FILTER_IN].name)
    vty_out (vty, "  Incoming update network filter list is %s%s%s",
	     filter->dlist[FILTER_IN].alist ? "*" : "",
	     filter->dlist[FILTER_IN].name,
	     VTY_NEWLINE);
  if (filter->dlist[FILTER_OUT].name)
    vty_out (vty, "  Outgoing update network filter list is %s%s%s",
	     filter->dlist[FILTER_OUT].alist ? "*" : "",
	     filter->dlist[FILTER_OUT].name,
	     VTY_NEWLINE);

  /* filter-list. */
  if (filter->aslist[FILTER_IN].name)
    vty_out (vty, "  Incoming update AS path filter list is %s%s%s",
	     filter->aslist[FILTER_IN].aslist ? "*" : "",
	     filter->aslist[FILTER_IN].name,
	     VTY_NEWLINE);
  if (filter->aslist[FILTER_OUT].name)
    vty_out (vty, "  Outgoing update AS path filter list is %s%s%s",
	     filter->aslist[FILTER_OUT].aslist ? "*" : "",
	     filter->aslist[FILTER_OUT].name,
	     VTY_NEWLINE);

  /* route-map. */
  if (filter->map[FILTER_IN].name)
    vty_out (vty, "  Route map for incoming advertisements is %s%s%s",
	     filter->map[FILTER_IN].map ? "*" : "",
	     filter->map[FILTER_IN].name,
	     VTY_NEWLINE);
  if (filter->map[FILTER_OUT].name)
    vty_out (vty, "  Route map for outgoing advertisements is %s%s%s",
	     filter->map[FILTER_OUT].map ? "*" : "",
	     filter->map[FILTER_OUT].name,
	     VTY_NEWLINE);

  vty_out (vty, "  %ld accepted prefixes",
	   conf->pcount[afi][safi]);
  if (conf->pmax[afi][safi])
    {
      vty_out (vty, ", maximum limit %ld%s",
	       conf->pmax[afi][safi],
	       conf->pmax_warning[afi][safi] ? " (warning-only)" : "");
    }
  vty_out (vty, "%s", VTY_NEWLINE);

  vty_out (vty, "%s", VTY_NEWLINE);
}

void
bgp_show_peer (struct vty *vty, struct peer_conf *conf, afi_t afi, safi_t safi)
{
  char buf1[BUFSIZ];
  char timebuf[BGP_UPTIME_LEN];
  struct peer *p;

  p = conf->peer;

  /* Configured IP address. */
  vty_out (vty, "BGP neighbor is %s, ", p->host);
  vty_out (vty, "remote AS %d, ", p->as);
  vty_out (vty, "local AS %d, ", p->local_as);
  vty_out (vty, "%s link%s",
	   p->as == p->local_as ? "internal" : "external",
	   VTY_NEWLINE);

  /* Description. */
  if (p->desc)
    vty_out (vty, " Description: %s%s", p->desc, VTY_NEWLINE);
  
  /* Administrative shutdown. */
  if (CHECK_FLAG (p->flags, PEER_FLAG_SHUTDOWN))
    vty_out (vty, " Administratively shut down%s", VTY_NEWLINE);

  /* BGP Version. */
  vty_out (vty, "  BGP version 4");
  if (p->version == BGP_VERSION_MP_4_DRAFT_00)
    vty_out (vty, "(with draft-00 verion of multiporotocol extension)");
  vty_out (vty, ", remote router ID %s%s", 
	   inet_ntop (AF_INET, &p->remote_id, buf1, BUFSIZ),
	   VTY_NEWLINE);

  /* Confederation */
  if (bgp_confederation_peers_check (conf->bgp, p->as)) 
    vty_out (vty, "  Neighbor under common administration%s", VTY_NEWLINE);
  
  /* Status. */
  vty_out (vty, "  BGP state = %s",  
	   CHECK_FLAG (p->flags, PEER_FLAG_SHUTDOWN) 
	   ? "Idle" : LOOKUP(bgp_status_msg, p->status));
  if (p->status == Established) 
    vty_out (vty, ", up for %8s", 
	     peer_uptime (p->uptime, timebuf, BGP_UPTIME_LEN));
  vty_out (vty, "%s", VTY_NEWLINE);
  
  /* read timer */
  vty_out (vty, "  Last read %s", peer_uptime (p->readtime, timebuf, BGP_UPTIME_LEN));

  /* Configured timer values. */
  vty_out (vty, ", hold time is %d, keepalive interval is %d seconds%s",
	   p->v_holdtime, p->v_keepalive, VTY_NEWLINE);
  if (p->config & PEER_CONFIG_TIMER)
    {
      vty_out (vty, "  Configured hold time is %d", p->holdtime);
      vty_out (vty, ", keepalive interval is %d seconds%s",
	       p->keepalive, VTY_NEWLINE);
    }
  
  /* Capability. */
  if (p->status == Established) 
    {
      if ((p->refresh_adv || p->refresh_nego_old || p->refresh_nego_new)
	  || (p->afc_adv[AFI_IP][SAFI_UNICAST] || p->afc_recv[AFI_IP][SAFI_UNICAST]) 
	  || (p->afc_adv[AFI_IP][SAFI_MULTICAST] || p->afc_recv[AFI_IP][SAFI_MULTICAST]) 
#ifdef HAVE_IPV6
	  || (p->afc_adv[AFI_IP6][SAFI_UNICAST] || p->afc_recv[AFI_IP6][SAFI_UNICAST]) 
	  || (p->afc_adv[AFI_IP6][SAFI_MULTICAST] || p->afc_recv[AFI_IP6][SAFI_MULTICAST]) 
#endif /* HAVE_IPV6 */
	  || (p->afc_adv[AFI_IP][SAFI_MPLS_VPN] || p->afc_recv[AFI_IP][SAFI_MPLS_VPN])) 
	{
	  vty_out (vty, "  Neighbor capabilities:%s", VTY_NEWLINE);

	  /* Route Refresh */
	  if (p->refresh_adv || p->refresh_nego_old || p->refresh_nego_new)
	    {
	      vty_out (vty, "    Route refresh:");
 	      if (p->refresh_adv)
		vty_out (vty, " advertised");
	      if (p->refresh_nego_old || p->refresh_nego_new)
		{
		  if (p->refresh_adv)
		    vty_out (vty, " and");
		  vty_out (vty, " received");
		  if (p->refresh_nego_old && p->refresh_nego_new)
		    vty_out (vty, " (old and new)");
		  else if (p->refresh_nego_old)
		    vty_out (vty, " (old)");
		  else 
		    vty_out (vty, " (new)");
		}
	      vty_out (vty, "%s", VTY_NEWLINE);
	    }

	  /* IPv4 */
	  if (p->afc_adv[AFI_IP][SAFI_UNICAST] || p->afc_recv[AFI_IP][SAFI_UNICAST]) 
	    {
	      vty_out (vty, "    Address family IPv4 Unicast:");
	      if (p->afc_adv[AFI_IP][SAFI_UNICAST]) 
		vty_out (vty, " advertised");
	      if (p->afc_recv[AFI_IP][SAFI_UNICAST])
		{
		  if (p->afc_adv[AFI_IP][SAFI_UNICAST])
		    vty_out (vty, " and");
		  vty_out (vty, " received");
		}
	      vty_out (vty, "%s", VTY_NEWLINE);
	    }
	  if (p->afc_adv[AFI_IP][SAFI_MULTICAST] || p->afc_recv[AFI_IP][SAFI_MULTICAST]) 
	    {
	      vty_out (vty, "    Address family IPv4 Multicast:");
	      if (p->afc_adv[AFI_IP][SAFI_MULTICAST]) 
		vty_out (vty, " advertised");
	      if (p->afc_recv[AFI_IP][SAFI_MULTICAST])
		{
		  if (p->afc_adv[AFI_IP][SAFI_MULTICAST])
		    vty_out (vty, " and");
		  vty_out (vty, " received");
		}
	      vty_out (vty, "%s", VTY_NEWLINE);
	    }
	  if (p->afc_adv[AFI_IP][SAFI_MPLS_VPN] || p->afc_recv[AFI_IP][SAFI_MPLS_VPN]) 
	    {
	      vty_out (vty, "    Address family VPNv4 Unicast:");
	      if (p->afc_adv[AFI_IP][SAFI_MPLS_VPN]) 
		vty_out (vty, " advertised");
	      if (p->afc_recv[AFI_IP][SAFI_MPLS_VPN])
		{
		  if (p->afc_adv[AFI_IP][SAFI_MPLS_VPN])
		    vty_out (vty, " and");
		  vty_out (vty, " received");
		}
	      vty_out (vty, "%s", VTY_NEWLINE);
	    }
	  /* IPv6 */
#ifdef HAVE_IPV6
	  if (p->afc_adv[AFI_IP6][SAFI_UNICAST] || p->afc_recv[AFI_IP6][SAFI_UNICAST]) 
	    {
	      vty_out (vty, "    Address family IPv6 Unicast:");
	      if (p->afc_adv[AFI_IP6][SAFI_UNICAST]) 
		vty_out (vty, " advertised");
	      if (p->afc_recv[AFI_IP6][SAFI_UNICAST])
		{
		  if (p->afc_adv[AFI_IP6][SAFI_UNICAST])
		    vty_out (vty, " and");
		  vty_out (vty, " received");
		}
	      vty_out (vty, "%s", VTY_NEWLINE);
	    }
	  if (p->afc_adv[AFI_IP6][SAFI_MULTICAST] || p->afc_recv[AFI_IP6][SAFI_MULTICAST]) 
	    {
	      vty_out (vty, "    Address family IPv6 Multicast:");
	      if (p->afc_adv[AFI_IP6][SAFI_MULTICAST]) 
		vty_out (vty, " advertised");
	      if (p->afc_recv[AFI_IP6][SAFI_MULTICAST])
		{
		  if (p->afc_adv[AFI_IP6][SAFI_MULTICAST])
		    vty_out (vty, " and");
		  vty_out (vty, " received");
		}
	      vty_out (vty, "%s", VTY_NEWLINE);
	    }
#endif /* HAVE_IPV6 */
	}
    }

  /* Packet counts. */
  vty_out(vty, "  Received %d messages, %d notifications, %d in queue%s",
	  p->open_in + p->update_in + p->keepalive_in,
	  p->notify_in, 0, VTY_NEWLINE);
  vty_out(vty, "  Sent %d messages, %d notifications, %ld in queue%s",
	  p->open_out + p->update_out + p->keepalive_out,
	  p->notify_out, p->obuf->count, VTY_NEWLINE);
  vty_out(vty, "  Route refresh request: received %d, sent %d%s",
	  p->refresh_in, p->refresh_out, VTY_NEWLINE);

  /* advertisement-interval (Current not supported)*/
  vty_out (vty, "  Minimum time between advertisement runs is 0 seconds%s",
	   VTY_NEWLINE);

  /* Default weight */
  if (p->weight)
    vty_out (vty, "  Default weight %d%s", p->weight,
	     VTY_NEWLINE);

  vty_out (vty, "%s", VTY_NEWLINE);

  /* Address Family Information */
  if (p->afc[AFI_IP][SAFI_UNICAST])
    bgp_show_peer_afi (vty, conf, AFI_IP, SAFI_UNICAST);
  if (p->afc[AFI_IP][SAFI_MULTICAST])
    bgp_show_peer_afi (vty, conf, AFI_IP, SAFI_MULTICAST);
  if (p->afc[AFI_IP][SAFI_MPLS_VPN])
    bgp_show_peer_afi (vty, conf, AFI_IP, SAFI_MPLS_VPN);
#ifdef HAVE_IPV6
  if (p->afc[AFI_IP6][SAFI_UNICAST])
    bgp_show_peer_afi (vty, conf, AFI_IP6, SAFI_UNICAST);
  if (p->afc[AFI_IP6][SAFI_MULTICAST])
    bgp_show_peer_afi (vty, conf, AFI_IP6, SAFI_MULTICAST);
#endif /* HAVE_IPV6 */

  vty_out (vty, "  Connections established %d; dropped %d%s",
	   p->established, p->dropped,
	   VTY_NEWLINE);

  if (CHECK_FLAG (p->sflags, PEER_STATUS_PREFIX_OVERFLOW))
    {
      vty_out (vty, "  Peer had exceeded the max. no. of prefixes configured.%s", VTY_NEWLINE);
      vty_out (vty, "  Reduce the no. of prefix and clear ip bgp %s to restore peering%s",
	       p->host, VTY_NEWLINE);
    }

  /* EBGP Multihop */
  if (peer_sort (p) == BGP_PEER_EBGP && p->ttl > 1)
    vty_out (vty, "  External BGP neighbor may be up to %d hops away.%s",
	     p->ttl, VTY_NEWLINE);
      
  /* Local address. */
  if (p->su_local)
    {
      vty_out (vty, "Local host: %s, Local port: %d%s",
	       sockunion2str (p->su_local, buf1, SU_ADDRSTRLEN),
	       ntohs (p->su_local->sin.sin_port),
	       VTY_NEWLINE);
    }
      
  /* Remote address. */
  if (p->su_remote)
    {
      vty_out (vty, "Foreign host: %s, Foreign port: %d%s",
	       sockunion2str (p->su_remote, buf1, SU_ADDRSTRLEN),
	       ntohs (p->su_remote->sin.sin_port),
	       VTY_NEWLINE);
    }

  /* Nexthop display. */
  if (p->su_local)
    {
      vty_out (vty, "Nexthop: %s%s", 
	       inet_ntop (AF_INET, &p->nexthop.v4, buf1, BUFSIZ),
	       VTY_NEWLINE);
#ifdef HAVE_IPV6
      vty_out (vty, "Nexthop global: %s%s", 
	       inet_ntop (AF_INET6, &p->nexthop.v6_global, buf1, BUFSIZ),
	       VTY_NEWLINE);
      vty_out (vty, "Nexthop local: %s%s",
	       inet_ntop (AF_INET6, &p->nexthop.v6_local, buf1, BUFSIZ),
	       VTY_NEWLINE);
      vty_out (vty, "BGP connection: %s%s",
	       p->shared_network ? "shared network" : "non shared network",
	       VTY_NEWLINE);
#endif /* HAVE_IPV6 */
    }

  /* Timer information. */
  if (p->t_start)
    vty_out (vty, "Next start timer due in %d seconds%s",
	     bgp_next_timer (p->t_start), VTY_NEWLINE);
  if (p->t_connect)
    vty_out (vty, "Next connect timer due in %d seconds%s",
	     bgp_next_timer (p->t_connect), VTY_NEWLINE);
  
  vty_out (vty, "Read thread: %s  Write thread: %s%s", 
	   p->t_read ? "on" : "off",
	   p->t_write ? "on" : "off",
	   VTY_NEWLINE);

  if (p->notify.code == BGP_NOTIFY_OPEN_ERR
      && p->notify.subcode == BGP_NOTIFY_OPEN_UNSUP_CAPBL)
    bgp_capability_vty_out (vty, p);
 
  vty_out (vty, "%s", VTY_NEWLINE);
}

int
bgp_show_neighbor (struct vty *vty, int afi, int safi, enum show_type type,
		   char *ip_str)
{
  struct listnode *nn, *nm;
  struct bgp *bgp;
  struct peer_conf *conf;
  union sockunion su;
  int ret;
  int find = 0;

  if (ip_str)
    {
      ret = str2sockunion (ip_str, &su);
      if (ret < 0)
	{
	  vty_out (vty, "Malformed address: %s%s", ip_str, VTY_NEWLINE);
	  return CMD_WARNING;
	}
    }

  LIST_LOOP (bgp_list, bgp, nn)
    {
      LIST_LOOP (bgp->peer_conf, conf, nm)
	{
	  switch (type)
	    {
	    case show_all:
	      if (conf->afc[afi][safi])
		bgp_show_peer (vty, conf, afi, safi);
	      break;
	    case show_peer:
	      if (conf->afc[afi][safi] 
		  && sockunion_same (&conf->peer->su, &su))
		{
		  find = 1;
		  bgp_show_peer (vty, conf, afi, safi);
		}
	      break;
	    }
	}
    }

  if (type == show_peer && ! find)
    vty_out (vty, "%% No such neighbor or address family%s", VTY_NEWLINE);
  
  return CMD_SUCCESS;
}

DEFUN (show_ip_bgp_neighbors,
       show_ip_bgp_neighbors_cmd,
       "show ip bgp neighbors",
       SHOW_STR
       IP_STR
       BGP_STR
       "Detailed information on TCP and BGP neighbor connections\n")
{
  return bgp_show_neighbor (vty, AFI_IP, SAFI_UNICAST, show_all, NULL);
}

DEFUN (show_ip_bgp_ipv4_neighbors,
       show_ip_bgp_ipv4_neighbors_cmd,
       "show ip bgp ipv4 (unicast|multicast) neighbors",
       SHOW_STR
       IP_STR
       BGP_STR
       "Address family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Detailed information on TCP and BGP neighbor connections\n")
{
  if (strncmp (argv[0], "m", 1) == 0)
    return bgp_show_neighbor (vty, AFI_IP, SAFI_MULTICAST, show_all, NULL);

  return bgp_show_neighbor (vty, AFI_IP, SAFI_UNICAST, show_all, NULL);
}

DEFUN (show_ip_bgp_neighbors_peer,
       show_ip_bgp_neighbors_peer_cmd,
       "show ip bgp neighbors (A.B.C.D|X:X::X:X)",
       SHOW_STR
       IP_STR
       BGP_STR
       "Detailed information on TCP and BGP neighbor connections\n"
       "Neighbor to display information about\n"
       "Neighbor to display information about\n")
{
  return bgp_show_neighbor (vty, AFI_IP, SAFI_UNICAST, show_peer, argv[0]);
}

DEFUN (show_ip_bgp_ipv4_neighbors_peer,
       show_ip_bgp_ipv4_neighbors_peer_cmd,
       "show ip bgp ipv4 (unicast|multicast) neighbors (A.B.C.D|X:X::X:X)",
       SHOW_STR
       IP_STR
       BGP_STR
       "Address family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Detailed information on TCP and BGP neighbor connections\n"
       "Neighbor to display information about\n"
       "Neighbor to display information about\n")
{
  if (strncmp (argv[0], "m", 1) == 0)
    return bgp_show_neighbor (vty, AFI_IP, SAFI_MULTICAST, show_peer, argv[1]);
 
  return bgp_show_neighbor (vty, AFI_IP, SAFI_UNICAST, show_peer, argv[1]);
}

DEFUN (show_ip_bgp_vpnv4_all_neighbors,
       show_ip_bgp_vpnv4_all_neighbors_cmd,
       "show ip bgp vpnv4 all neighbors",
       SHOW_STR
       IP_STR
       BGP_STR
       "Display VPNv4 NLRI specific information\n"
       "Display information about all VPNv4 NLRIs\n"
       "Detailed information on TCP and BGP neighbor connections\n")
{
  return bgp_show_neighbor (vty, AFI_IP, SAFI_MPLS_VPN, show_all, NULL);
}

DEFUN (show_ip_bgp_vpnv4_rd_neighbors,
       show_ip_bgp_vpnv4_rd_neighbors_cmd,
       "show ip bgp vpnv4 rd ASN:nn_or_IP-address:nn neighbors",
       SHOW_STR
       IP_STR
       BGP_STR
       "Display VPNv4 NLRI specific information\n"
       "Display information for a route distinguisher\n"
       "VPN Route Distinguisher\n"
       "Detailed information on TCP and BGP neighbor connections\n")
{
  int ret;
  struct prefix_rd prd;

  ret = str2prefix_rd (argv[0], &prd);
  if (! ret)
    {
      vty_out (vty, "%% Malformed Route Distinguisher%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  return bgp_show_neighbor (vty, AFI_IP, SAFI_MPLS_VPN, show_all, NULL);
}

DEFUN (show_ip_bgp_vpnv4_all_neighbors_peer,
       show_ip_bgp_vpnv4_all_neighbors_peer_cmd,
       "show ip bgp vpnv4 all neighbors A.B.C.D",
       SHOW_STR
       IP_STR
       BGP_STR
       "Display VPNv4 NLRI specific information\n"
       "Display information about all VPNv4 NLRIs\n"
       "Detailed information on TCP and BGP neighbor connections\n"
       "Neighbor to display information about\n")
{
  return bgp_show_neighbor (vty, AFI_IP, SAFI_MPLS_VPN, show_peer, argv[0]);
}

DEFUN (show_ip_bgp_vpnv4_rd_neighbors_peer,
       show_ip_bgp_vpnv4_rd_neighbors_peer_cmd,
       "show ip bgp vpnv4 rd ASN:nn_or_IP-address:nn neighbors A.B.C.D",
       SHOW_STR
       IP_STR
       BGP_STR
       "Display VPNv4 NLRI specific information\n"
       "Display information about all VPNv4 NLRIs\n"
       "Detailed information on TCP and BGP neighbor connections\n"
       "Neighbor to display information about\n")
{
  int ret;
  struct prefix_rd prd;

  ret = str2prefix_rd (argv[0], &prd);
  if (! ret)
    {
      vty_out (vty, "%% Malformed Route Distinguisher%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  return bgp_show_neighbor (vty, AFI_IP, SAFI_MPLS_VPN, show_peer, argv[1]);
}

#ifdef HAVE_IPV6
DEFUN (show_bgp_neighbors,
       show_bgp_neighbors_cmd,
       "show bgp neighbors",
       SHOW_STR
       BGP_STR
       "Detailed information on TCP and BGP neighbor connections\n")
{
  return bgp_show_neighbor (vty, AFI_IP6, SAFI_UNICAST, show_all, NULL);
}

ALIAS (show_bgp_neighbors,
       show_bgp_ipv6_neighbors_cmd,
       "show bgp ipv6 neighbors",
       SHOW_STR
       BGP_STR
       "Address family\n"
       "Detailed information on TCP and BGP neighbor connections\n")

/* old command */
DEFUN (show_ipv6_bgp_neighbors,
       show_ipv6_bgp_neighbors_cmd,
       "show ipv6 bgp neighbors",
       SHOW_STR
       IPV6_STR
       BGP_STR
       "Detailed information on TCP and BGP neighbor connections\n")
{
  return bgp_show_neighbor (vty, AFI_IP6, SAFI_UNICAST, show_all, NULL);
}

DEFUN (show_bgp_neighbors_peer,
       show_bgp_neighbors_peer_cmd,
       "show bgp neighbors (A.B.C.D|X:X::X:X)",
       SHOW_STR
       BGP_STR
       "Detailed information on TCP and BGP neighbor connections\n"
       "Neighbor to display information about\n"
       "Neighbor to display information about\n")
{
  return bgp_show_neighbor (vty, AFI_IP6, SAFI_UNICAST, show_peer, argv[0]);
}

ALIAS (show_bgp_neighbors_peer,
       show_bgp_ipv6_neighbors_peer_cmd,
       "show bgp ipv6 neighbors (A.B.C.D|X:X::X:X)",
       SHOW_STR
       BGP_STR
       "Address family\n"
       "Detailed information on TCP and BGP neighbor connections\n"
       "Neighbor to display information about\n"
       "Neighbor to display information about\n")

/* old command */
DEFUN (show_ipv6_bgp_neighbors_peer,
       show_ipv6_bgp_neighbors_peer_cmd,
       "show ipv6 bgp neighbors (A.B.C.D|X:X::X:X)",
       SHOW_STR
       IPV6_STR
       BGP_STR
       "Detailed information on TCP and BGP neighbor connections\n"
       "Neighbor to display information about\n"
       "Neighbor to display information about\n")
{
  return bgp_show_neighbor (vty, AFI_IP6, SAFI_UNICAST, show_peer, argv[0]);
}

/* old command */
DEFUN (show_ipv6_mbgp_neighbors,
       show_ipv6_mbgp_neighbors_cmd,
       "show ipv6 mbgp neighbors",
       SHOW_STR
       IPV6_STR
       MBGP_STR
       "Detailed information on TCP and BGP neighbor connections\n")
{
  return bgp_show_neighbor (vty, AFI_IP6, SAFI_MULTICAST, show_all, NULL);
}

/* old command */
DEFUN (show_ipv6_mbgp_neighbors_peer,
       show_ipv6_mbgp_neighbors_peer_cmd,
       "show ipv6 mbgp neighbors (A.B.C.D|X:X::X:X)",
       SHOW_STR
       IPV6_STR
       MBGP_STR
       "Detailed information on TCP and BGP neighbor connections\n"
       "Neighbor to display information about\n"
       "Neighbor to display information about\n")
{
  return bgp_show_neighbor (vty, AFI_IP6, SAFI_MULTICAST, show_peer, argv[0]);
}
#endif /* HAVE_IPV6 */

/* Show BGP's AS paths internal data.  There are both `show ip bgp
   paths' and `show ip mbgp paths'.  Those functions results are the
   same.*/
DEFUN (show_ip_bgp_paths, 
       show_ip_bgp_paths_cmd,
       "show ip bgp paths",
       SHOW_STR
       IP_STR
       BGP_STR
       "Path information\n")
{
  vty_out (vty, "Address Refcnt Path%s", VTY_NEWLINE);
  aspath_print_all_vty (vty);
  return CMD_SUCCESS;
}

DEFUN (show_ip_bgp_ipv4_paths, 
       show_ip_bgp_ipv4_paths_cmd,
       "show ip bgp ipv4 (unicast|multicast) paths",
       SHOW_STR
       IP_STR
       BGP_STR
       "Address family\n"
       "Address Family modifier\n"
       "Address Family modifier\n"
       "Path information\n")
{
  vty_out (vty, "Address Refcnt Path\r\n");
  aspath_print_all_vty (vty);

  return CMD_SUCCESS;
}

/* Show BGP's community internal data. */
DEFUN (show_ip_bgp_community_info, 
       show_ip_bgp_community_info_cmd,
       "show ip bgp community-info",
       SHOW_STR
       IP_STR
       BGP_STR
       "List all bgp community information\n")
{
  vty_out (vty, "Address Refcnt Community%s", VTY_NEWLINE);
  community_print_all_vty (vty);
  return CMD_SUCCESS;
}

DEFUN (show_ip_bgp_attr_info, 
       show_ip_bgp_attr_info_cmd,
       "show ip bgp attribute-info",
       SHOW_STR
       IP_STR
       BGP_STR
       "List all bgp attribute information\n")
{
  void attrhash_dump (struct vty *);

  attrhash_dump (vty);
  return CMD_SUCCESS;
}

void
bgp_config_write_filter (struct vty *vty, struct bgp_filter *filter,
			 char *addr)
{
  /* distribute-list. */
  if (filter->dlist[FILTER_IN].name)
    vty_out (vty, " neighbor %s distribute-list %s in%s", addr, 
	     filter->dlist[FILTER_IN].name, VTY_NEWLINE);
  if (filter->dlist[FILTER_OUT].name)
    vty_out (vty, " neighbor %s distribute-list %s out%s", addr, 
	     filter->dlist[FILTER_OUT].name, VTY_NEWLINE);

  /* prefix-list. */
  if (filter->plist[FILTER_IN].name)
    vty_out (vty, " neighbor %s prefix-list %s in%s", addr, 
	     filter->plist[FILTER_IN].name, VTY_NEWLINE);
  if (filter->plist[FILTER_OUT].name)
    vty_out (vty, " neighbor %s prefix-list %s out%s", addr, 
	     filter->plist[FILTER_OUT].name, VTY_NEWLINE);

  /* filter-list. */
  if (filter->aslist[FILTER_IN].name)
    vty_out (vty, " neighbor %s filter-list %s in%s", addr, 
	     filter->aslist[FILTER_IN].name, VTY_NEWLINE);
  if (filter->aslist[FILTER_OUT].name)
    vty_out (vty, " neighbor %s filter-list %s out%s", addr, 
	     filter->aslist[FILTER_OUT].name, VTY_NEWLINE);

  /* route-map. */
  if (filter->map[FILTER_IN].name)
    vty_out (vty, " neighbor %s route-map %s in%s", addr, 
	     filter->map[FILTER_IN].name, VTY_NEWLINE);
  if (filter->map[FILTER_OUT].name)
    vty_out (vty, " neighbor %s route-map %s out%s", addr, 
	     filter->map[FILTER_OUT].name, VTY_NEWLINE);
}

/* BGP peer configuration display function. */
void
bgp_config_write_peer (struct vty *vty, struct bgp *bgp,
		       struct peer_conf *conf, afi_t afi, safi_t safi)
{
  struct peer *peer;
  char addr[SU_ADDRSTRLEN];
  char buf[SU_ADDRSTRLEN];
  struct bgp_filter *filter;

  peer = conf->peer;
  filter = &conf->filter[afi][safi];
  sockunion2str (&peer->su, addr, SU_ADDRSTRLEN);

  /************************************
   ****** Global to the neighbor ******
   ************************************/
  if (afi == AFI_IP && safi == SAFI_UNICAST)
    {
      /* remote-as. */
      vty_out (vty, " neighbor %s remote-as %d%s", addr, peer->as,
	       VTY_NEWLINE);

      /* Description. */
      if (peer->desc)
	vty_out (vty, " neighbor %s description %s%s", addr, peer->desc,
		 VTY_NEWLINE);

      /* Shutdown. */
      if (CHECK_FLAG (peer->flags, PEER_FLAG_SHUTDOWN))
	vty_out (vty, " neighbor %s shutdown%s", addr, VTY_NEWLINE);

      /* BGP port. */
      if (peer->port != BGP_PORT_DEFAULT)
	vty_out (vty, " neighbor %s port %d%s", addr, peer->port, 
		 VTY_NEWLINE);

      /* Local interface name. */
      if (peer->ifname)
	vty_out (vty, " neighbor %s interface %s%s", addr, peer->ifname,
		 VTY_NEWLINE);
  
      /* Passive. */
      if (CHECK_FLAG (peer->flags, PEER_FLAG_PASSIVE))
	vty_out (vty, " neighbor %s passive%s", addr, VTY_NEWLINE);

      /* ebgp-multihop print. */
      if (peer_sort (peer) == BGP_PEER_EBGP && peer->ttl != 1)
	vty_out (vty, " neighbor %s ebgp-multihop %d%s", addr, peer->ttl,
		 VTY_NEWLINE);

      /* Update-source. */
      if (peer->update_if)
	vty_out (vty, " neighbor %s update-source %s%s", addr, 
		 peer->update_if, VTY_NEWLINE);
      if (peer->update_source)
	vty_out (vty, " neighbor %s update-source %s%s", addr, 
		 sockunion2str (peer->update_source, buf, SU_ADDRSTRLEN),
		 VTY_NEWLINE);

      /* BGP version print. */
      if (peer->version == BGP_VERSION_MP_4_DRAFT_00)
	vty_out (vty, " neighbor %s version %s%s",
		 addr,"4-", VTY_NEWLINE);

      /* timers. */
      if (peer->config & PEER_CONFIG_TIMER)
	  vty_out (vty, " neighbor %s timers %d %d%s", addr, 
	  peer->keepalive, peer->holdtime, VTY_NEWLINE);
      if (peer->config & PEER_CONFIG_CONNECT)
	  vty_out (vty, " neighbor %s timers connect %d%s", addr, 
	  peer->connect, VTY_NEWLINE);

      /* Default weight. */
      if (peer->weight)
	vty_out (vty, " neighbor %s weight %d%s", addr, peer->weight,
		 VTY_NEWLINE);

      /* Route refresh. */
      if (! CHECK_FLAG (peer->flags, PEER_FLAG_CAPABILITY_ROUTE_REFRESH))
	  vty_out (vty, " no neighbor %s capability route-refresh%s", addr,
	  VTY_NEWLINE);

      /* dont capability negotiation. */
      if (CHECK_FLAG (peer->flags, PEER_FLAG_DONT_CAPABILITY))
	vty_out (vty, " neighbor %s dont-capability-negotiate%s", addr,
		 VTY_NEWLINE);

      /* override capability negotiation. */
      if (CHECK_FLAG (peer->flags, PEER_FLAG_OVERRIDE_CAPABILITY))
	vty_out (vty, " neighbor %s override-capability%s", addr,
		 VTY_NEWLINE);

      /* strict capability negotiation. */
      if (CHECK_FLAG (peer->flags, PEER_FLAG_STRICT_CAP_MATCH))
	vty_out (vty, " neighbor %s strict-capability-match%s", addr,
	     VTY_NEWLINE);

      if (CHECK_FLAG (bgp->config, BGP_CONFIG_NO_DEFAULT_IPV4))
	{
	  if (conf->afc[AFI_IP][SAFI_UNICAST])
	    vty_out (vty, " neighbor %s activate%s", addr, VTY_NEWLINE);
	}
      else
	{
	  if (! conf->afc[AFI_IP][SAFI_UNICAST])
	    vty_out (vty, " no neighbor %s activate%s", addr, VTY_NEWLINE);
	}
    }


  /************************************
   ****** Per AF to the neighbor ******
   ************************************/

  if (! (afi == AFI_IP && safi == SAFI_UNICAST))
    vty_out (vty, " neighbor %s activate%s", addr, VTY_NEWLINE);

  /* Route reflector client. */
  if (CHECK_FLAG (peer->af_flags[afi][safi], PEER_FLAG_REFLECTOR_CLIENT))
    vty_out (vty, " neighbor %s route-reflector-client%s", addr, VTY_NEWLINE);

  /* Nexthop self. */
  if (CHECK_FLAG (peer->af_flags[afi][safi], PEER_FLAG_NEXTHOP_SELF))
    vty_out (vty, " neighbor %s next-hop-self%s", addr, VTY_NEWLINE);

  /* send-community print. */
  if (! (CHECK_FLAG (peer->af_flags[afi][safi], PEER_FLAG_SEND_COMMUNITY)
         && CHECK_FLAG (peer->af_flags[afi][safi], PEER_FLAG_SEND_EXT_COMMUNITY)))
    {
      if (! CHECK_FLAG (peer->af_flags[afi][safi], PEER_FLAG_SEND_COMMUNITY)
          && ! CHECK_FLAG (peer->af_flags[afi][safi], PEER_FLAG_SEND_EXT_COMMUNITY))
	vty_out (vty, " no neighbor %s send-community both%s",
		 addr, VTY_NEWLINE);
      else if (! CHECK_FLAG (peer->af_flags[afi][safi], PEER_FLAG_SEND_EXT_COMMUNITY))	
	vty_out (vty, " no neighbor %s send-community extended%s",
		 addr, VTY_NEWLINE);
      else
	vty_out (vty, " no neighbor %s send-community%s",
		 addr, VTY_NEWLINE);
    }

  /* Default information */
  if (CHECK_FLAG (peer->flags, PEER_FLAG_DEFAULT_ORIGINATE)
      && (afi == AFI_IP && safi == SAFI_UNICAST))
    vty_out (vty, " neighbor %s default-originate%s", addr, VTY_NEWLINE);

  /* Soft reconfiguration inbound. */
  if (CHECK_FLAG (peer->af_flags[afi][safi], PEER_FLAG_SOFT_RECONFIG))
    vty_out (vty, " neighbor %s soft-reconfiguration inbound%s", addr,
	     VTY_NEWLINE);

  /* maximum-prefix. */
  if (conf->pmax[afi][safi])
    vty_out (vty, " neighbor %s maximum-prefix %ld%s%s",
	     addr, conf->pmax[afi][safi],
	     conf->pmax_warning[afi][safi] ? " warning-only" : "",
	     VTY_NEWLINE);

  /* Route server client. */
  if (CHECK_FLAG (peer->af_flags[afi][safi], PEER_FLAG_RSERVER_CLIENT))
    vty_out (vty, " neighbor %s route-server-client%s", addr, VTY_NEWLINE);

  /* translate-update. */
  if (peer->translate_update
      && (afi == AFI_IP && safi == SAFI_UNICAST))
    {
      vty_out (vty, " neighbor %s", addr);

      if (peer->translate_update == SAFI_UNICAST_MULTICAST) 
	vty_out (vty, " translate-update nlri unicast multicast%s", 
		 VTY_NEWLINE);
      else if (peer->translate_update == SAFI_MULTICAST) 
	vty_out (vty, " translate-update nlri multicast%s", 
		 VTY_NEWLINE);
    }

  /* Filter. */
  bgp_config_write_filter (vty, filter, addr);

  /* transparent-as. */
  if (CHECK_FLAG (peer->flags, PEER_FLAG_TRANSPARENT_AS)
      && (afi == AFI_IP && safi == SAFI_UNICAST))
    vty_out (vty, " neighbor %s transparent-as%s", addr, VTY_NEWLINE);

  /* transparent-nexthop. */
  if (CHECK_FLAG (peer->flags, PEER_FLAG_TRANSPARENT_NEXTHOP)
      && (afi == AFI_IP && safi == SAFI_UNICAST))
    vty_out (vty, " neighbor %s transparent-nexthop%s", addr,
	     VTY_NEWLINE);
}

/* Display "address-family" configuration header. */
void
bgp_config_write_family_header (struct vty *vty, afi_t afi, safi_t safi,
				int *write)
{
  if (*write)
    return;

  if (afi == AFI_IP && safi == SAFI_UNICAST)
    return;

  vty_out (vty, "!%s address-family ", VTY_NEWLINE);

  if (afi == AFI_IP)
    {
      if (safi == SAFI_MULTICAST)
	vty_out (vty, "ipv4 multicast");
      else if (safi == SAFI_MPLS_VPN)
	vty_out (vty, "vpnv4 unicast");
    }
  else if (afi == AFI_IP6)
    vty_out (vty, "ipv6");

  vty_out (vty, "%s", VTY_NEWLINE);

  *write = 1;
}

/* Address family based peer configuration display.  */
int
bgp_config_write_family (struct vty *vty, struct bgp *bgp, afi_t afi,
			 safi_t safi)
{
  int write = 0;
  struct peer_conf *conf;
  struct listnode *nn;

  bgp_config_write_network (vty, bgp, afi, safi, &write);

  bgp_config_write_redistribute (vty, bgp, afi, safi, &write);

  LIST_LOOP (bgp->peer_conf, conf, nn)
    {
      if (conf->afc[afi][safi])
	{
	  bgp_config_write_family_header (vty, afi, safi, &write);
	  bgp_config_write_peer (vty, bgp, conf, afi, safi);
	}
    }
  if (write)
    vty_out (vty, " exit-address-family%s", VTY_NEWLINE);

  return write;
}

int
bgp_config_write (struct vty *vty)
{
  int write = 0;
  struct bgp *bgp;
  struct peer_group *group;
  struct peer_conf *conf;
  struct listnode *nn, *nm, *no;

  /* BGP Multiple instance. */
  if (bgp_multiple_instance)
    {    
      vty_out (vty, "bgp multiple-instance%s", VTY_NEWLINE);
      vty_out (vty, "!%s", VTY_NEWLINE);
    }

  /* BGP configuration. */
  LIST_LOOP (bgp_list, bgp, nn)
    {
      if (write)
	vty_out (vty, "!%s", VTY_NEWLINE);

      /* Router bgp ASN */
      vty_out (vty, "router bgp %d", bgp->as);

      if (bgp_multiple_instance)
	{
	  if (bgp->name)
	    vty_out (vty, " view %s", bgp->name);
	}
      vty_out (vty, "%s", VTY_NEWLINE);

      /* BGP router ID. */
      if (CHECK_FLAG (bgp->config, BGP_CONFIG_ROUTER_ID))
	vty_out (vty, " bgp router-id %s%s", inet_ntoa (bgp->id), 
		 VTY_NEWLINE);

      /* BGP configuration. */
      if (CHECK_FLAG (bgp->config, BGP_CONFIG_ALWAYS_COMPARE_MED))
	vty_out (vty, " bgp always-compare-med%s", VTY_NEWLINE);

      /* BGP default ipv4-unicast. */
      if (CHECK_FLAG (bgp->config, BGP_CONFIG_NO_DEFAULT_IPV4))
	vty_out (vty, " no bgp default ipv4-unicast%s", VTY_NEWLINE);

      /* BGP default local-preference. */
      if (bgp->default_local_pref != BGP_DEFAULT_LOCAL_PREF)
	vty_out (vty, " bgp default local-preference %d%s",
		 bgp->default_local_pref,
		 VTY_NEWLINE);
      
      /* BGP client-to-client reflection. */
      if (CHECK_FLAG (bgp->config, BGP_CONFIG_NO_CLIENT_TO_CLIENT))
	vty_out (vty, " no bgp client-to-client reflection%s", VTY_NEWLINE);
      
      /* BGP cluster ID. */
      if (CHECK_FLAG (bgp->config, BGP_CONFIG_CLUSTER_ID))
	vty_out (vty, " bgp cluster-id %s%s", inet_ntoa (bgp->cluster), 
		 VTY_NEWLINE);

      /* Confederation Information */
      if(CHECK_FLAG(bgp->config, BGP_CONFIG_CONFEDERATION))
	{
	  vty_out(vty, " bgp confederation identifier %i%s", 
		  bgp->confederation_id,
		  VTY_NEWLINE);
	  if(bgp->confederation_peers_cnt > 0)
	    {
	      vty_out(vty, " bgp confederation peers");
	      bgp_confederation_peers_print(vty, bgp);
	      vty_out(vty, "%s", VTY_NEWLINE);
	    }
	}

      /* BGP enforce-first-as. */
      if (CHECK_FLAG (bgp->config, BGP_CONFIG_ENFORCE_FIRST_AS))
	vty_out (vty, " bgp enforce-first-as%s", VTY_NEWLINE);

      /* BGP deterministic-med. */
      if (CHECK_FLAG (bgp->config, BGP_CONFIG_DETERMINISTIC_MED))
	vty_out (vty, " bgp deterministic-med%s", VTY_NEWLINE);
      
      /* BGP bestpath method. */
      if (CHECK_FLAG (bgp->config, BGP_CONFIG_ASPATH_IGNORE))
	vty_out (vty, " bgp bestpath as-path ignore%s", VTY_NEWLINE);
      if (CHECK_FLAG (bgp->config, BGP_CONFIG_COMPARE_ROUTER_ID))
	vty_out (vty, " bgp bestpath compare-routerid%s", VTY_NEWLINE);
      if (CHECK_FLAG (bgp->config, BGP_CONFIG_MED_CONFED)
	  || CHECK_FLAG (bgp->config, BGP_CONFIG_MED_MISSING_AS_WORST))
	{
	  vty_out (vty, " bgp bestpath med");
	  if (CHECK_FLAG (bgp->config, BGP_CONFIG_MED_CONFED))
	    vty_out (vty, " confed");
	  if (CHECK_FLAG (bgp->config, BGP_CONFIG_MED_MISSING_AS_WORST))
	    vty_out (vty, " missing-as-worst");
	  vty_out (vty, "%s", VTY_NEWLINE);
	}

      /* BGP network import check. */
      if (CHECK_FLAG (bgp->config, BGP_CONFIG_IMPORT_CHECK))
	vty_out (vty, " bgp network import-check%s", VTY_NEWLINE);

      /* BGP scan interval. */
      bgp_config_write_scan_time (vty);

      /* BGP flag dampening. */
      bgp_config_write_damp (vty);

      /* BGP static route configuration. */
      bgp_config_write_network (vty, bgp, AFI_IP, SAFI_UNICAST, &write);

      /* BGP redistribute configuration. */
      bgp_config_write_redistribute (vty, bgp, AFI_IP, SAFI_UNICAST, &write);

      /* BGP timers configuration. */
      if (bgp->default_keepalive != BGP_DEFAULT_KEEPALIVE
	  && bgp->default_holdtime != BGP_DEFAULT_HOLDTIME)
	vty_out (vty, " timers bgp %d %d%s", bgp->default_keepalive, 
		 bgp->default_holdtime, VTY_NEWLINE);

      /* peer-group */
      LIST_LOOP (bgp->peer_group, group, nm)
	{
	  vty_out (vty, " neighbor %s peer-group", group->name);
	  vty_out (vty, "%s", VTY_NEWLINE);

	  if (group->as)
	    vty_out (vty, " remote-as %d%s", group->as, VTY_NEWLINE);
	}

      /* Normal neighbor configuration. */
      LIST_LOOP (bgp->peer_conf, conf, no)
	{
	  bgp_config_write_peer (vty, bgp, conf, AFI_IP, SAFI_UNICAST);
	}

      /* Distance configuration. */
      bgp_config_write_distance (vty, bgp);
      
      /* IPv4 multicast configuration.  */
      write += bgp_config_write_family (vty, bgp, AFI_IP, SAFI_MULTICAST);

      /* IPv4 VPN configuration.  */
      write += bgp_config_write_family (vty, bgp, AFI_IP, SAFI_MPLS_VPN);

      /* IPv6 unicast configuration.  */
      write += bgp_config_write_family (vty, bgp, AFI_IP6, SAFI_UNICAST);

      write++;
    }
  return write;
}

/* BGP node structure. */
struct cmd_node bgp_node =
{
  BGP_NODE,
  "%s(config-router)# ",
  1,
};

struct cmd_node bgp_ipv4_multicast_node =
{
  BGP_IPV4M_NODE,
  "%s(config-router-af)# ",
  1,
};

struct cmd_node bgp_ipv6_unicast_node = 
{
  BGP_IPV6_NODE,
  "%s(config-router-af)# ",
  1,
};

/* Install bgp related commands. */
void
bgp_init ()
{
  /* Install bgp top node. */
  install_node (&bgp_node, bgp_config_write);
  install_node (&bgp_ipv4_multicast_node, NULL);
  install_node (&bgp_ipv6_unicast_node, NULL);

  install_default (BGP_NODE);
  install_default (BGP_IPV4M_NODE);
  install_default (BGP_IPV6_NODE);

  /* "bgp multiple-instance" commands. */
  install_element (CONFIG_NODE, &bgp_multiple_instance_cmd);
  install_element (CONFIG_NODE, &no_bgp_multiple_instance_cmd);

  /* "bgp router-id" commands. */
  install_element (BGP_NODE, &bgp_router_id_cmd);
  install_element (BGP_NODE, &no_bgp_router_id_cmd);
  install_element (BGP_NODE, &no_bgp_router_id_val_cmd);

  /* "timers bgp" commands. */
  install_element (BGP_NODE, &bgp_timers_cmd);
  install_element (BGP_NODE, &no_bgp_timers_cmd);

  /* "bgp cluster-id" commands. */
  install_element (BGP_NODE, &bgp_cluster_id_cmd);
  install_element (BGP_NODE, &bgp_cluster_id32_cmd);
  install_element (BGP_NODE, &no_bgp_cluster_id_cmd);
  install_element (BGP_NODE, &no_bgp_cluster_id_val_cmd);

  /* "bgp client-to-client reflection" commands */
  install_element (BGP_NODE, &no_bgp_client_to_client_reflection_cmd);
  install_element (BGP_NODE, &bgp_client_to_client_reflection_cmd);
  
  /* "bgp always-compare-med" commands */
  install_element (BGP_NODE, &bgp_always_compare_med_cmd);
  install_element (BGP_NODE, &no_bgp_always_compare_med_cmd);
  
  /* "bgp deterministic-med" commands */
  install_element (BGP_NODE, &bgp_deterministic_med_cmd);
  install_element (BGP_NODE, &no_bgp_deterministic_med_cmd);
 
  /* "bgp enforce-first-as" commands */
  install_element (BGP_NODE, &bgp_enforce_first_as_cmd);
  install_element (BGP_NODE, &no_bgp_enforce_first_as_cmd);

  /* "bgp bestpath compare-routerid" commands */
  install_element (BGP_NODE, &bgp_bestpath_compare_router_id_cmd);
  install_element (BGP_NODE, &no_bgp_bestpath_compare_router_id_cmd);

  /* "bgp bestpath as-path ignore" commands */
  install_element (BGP_NODE, &bgp_bestpath_aspath_ignore_cmd);
  install_element (BGP_NODE, &no_bgp_bestpath_aspath_ignore_cmd);

  /* "bgp bestpath med" commands */
  install_element (BGP_NODE, &bgp_bestpath_med_cmd);
  install_element (BGP_NODE, &bgp_bestpath_med2_cmd);
  install_element (BGP_NODE, &bgp_bestpath_med3_cmd);
  install_element (BGP_NODE, &no_bgp_bestpath_med_cmd);
  install_element (BGP_NODE, &no_bgp_bestpath_med2_cmd);
  install_element (BGP_NODE, &no_bgp_bestpath_med3_cmd);

  /* "bgp network import-check" commands. */
  install_element (BGP_NODE, &bgp_network_import_check_cmd);
  install_element (BGP_NODE, &no_bgp_network_import_check_cmd);

  /* "no bgp default ipv4-unicast" commands. */
  install_element (BGP_NODE, &no_bgp_default_ipv4_unicast_cmd);
  install_element (BGP_NODE, &bgp_default_ipv4_unicast_cmd);
  
  /* "bgp default local-preference" commands. */
  install_element (BGP_NODE, &bgp_default_local_preference_cmd);
  install_element (BGP_NODE, &no_bgp_default_local_preference_cmd);
  install_element (BGP_NODE, &no_bgp_default_local_preference_val_cmd);

  /* "router bgp" commands. */
  install_element (CONFIG_NODE, &router_bgp_cmd);
  install_element (CONFIG_NODE, &router_bgp_view_cmd);

  /* "no router bgp" commands. */
  install_element (CONFIG_NODE, &no_router_bgp_cmd);
  install_element (CONFIG_NODE, &no_router_bgp_view_cmd);

  /* "neighbor remote-as" commands. */
  install_element (BGP_NODE, &neighbor_remote_as_cmd);
  install_element (BGP_NODE, &neighbor_remote_as_unicast_cmd);
  install_element (BGP_NODE, &neighbor_remote_as_multicast_cmd);
  install_element (BGP_NODE, &neighbor_remote_as_unicast_multicast_cmd);

  install_element (BGP_NODE, &neighbor_activate_cmd);
  install_element (BGP_NODE, &no_neighbor_activate_cmd);
  install_element (BGP_IPV4M_NODE, &neighbor_activate_cmd);
  install_element (BGP_IPV4M_NODE, &no_neighbor_activate_cmd);
  install_element (BGP_IPV6_NODE, &neighbor_activate_cmd);
  install_element (BGP_IPV6_NODE, &no_neighbor_activate_cmd);

  /* "no neighbor remote-as" commands. */
  install_element (BGP_NODE, &no_neighbor_cmd);
  install_element (BGP_NODE, &no_neighbor_remote_as_cmd);

  /* "neighbor passive" commands. */
  install_element (BGP_NODE, &neighbor_passive_cmd);
  install_element (BGP_NODE, &no_neighbor_passive_cmd);

  /* "neighbor shutdown" commands. */
  install_element (BGP_NODE, &neighbor_shutdown_cmd);
  install_element (BGP_NODE, &no_neighbor_shutdown_cmd);

  /* "neighbor ebgp-multihop" commands. */
  install_element (BGP_NODE, &neighbor_ebgp_multihop_cmd);
  install_element (BGP_NODE, &neighbor_ebgp_multihop_ttl_cmd);
  install_element (BGP_NODE, &no_neighbor_ebgp_multihop_cmd);
  install_element (BGP_NODE, &no_neighbor_ebgp_multihop_ttl_cmd);

  /* "neighbor description" commands. */
  install_element (BGP_NODE, &neighbor_description_cmd);
  install_element (BGP_NODE, &no_neighbor_description_cmd);
  install_element (BGP_NODE, &no_neighbor_description_val_cmd);

  /* "neighbor version" commands. */
  install_element (BGP_NODE, &neighbor_version_cmd);
  install_element (BGP_NODE, &no_neighbor_version_cmd);

  /* "neighbor interface" commands. */
  install_element (BGP_NODE, &neighbor_interface_cmd);
  install_element (BGP_NODE, &no_neighbor_interface_cmd);

  /* "neighbor next-hop-self" commands. */
  install_element (BGP_NODE, &neighbor_nexthop_self_cmd);
  install_element (BGP_NODE, &no_neighbor_nexthop_self_cmd);
  install_element (BGP_IPV4M_NODE, &neighbor_nexthop_self_cmd);
  install_element (BGP_IPV4M_NODE, &no_neighbor_nexthop_self_cmd);
  install_element (BGP_IPV6_NODE, &neighbor_nexthop_self_cmd);
  install_element (BGP_IPV6_NODE, &no_neighbor_nexthop_self_cmd);

  /* "neighbor update-source" commands. "*/
  install_element (BGP_NODE, &neighbor_update_source_cmd);
  install_element (BGP_NODE, &no_neighbor_update_source_cmd);

  /* "neighbor default-originate" commands. */
  install_element (BGP_NODE, &neighbor_default_originate_cmd);
  install_element (BGP_NODE, &no_neighbor_default_originate_cmd);

  /* "neighbor port" commands. */
  install_element (BGP_NODE, &neighbor_port_cmd);
  install_element (BGP_NODE, &no_neighbor_port_cmd);
  install_element (BGP_NODE, &no_neighbor_port_val_cmd);

  /* "neighbor send-community" commands.*/
  install_element (BGP_NODE, &neighbor_send_community_cmd);
  install_element (BGP_NODE, &neighbor_send_community_type_cmd);
  install_element (BGP_NODE, &no_neighbor_send_community_cmd);
  install_element (BGP_NODE, &no_neighbor_send_community_type_cmd);
  install_element (BGP_IPV4M_NODE, &neighbor_send_community_cmd);
  install_element (BGP_IPV4M_NODE, &neighbor_send_community_type_cmd);
  install_element (BGP_IPV4M_NODE, &no_neighbor_send_community_cmd);
  install_element (BGP_IPV4M_NODE, &no_neighbor_send_community_type_cmd);
  install_element (BGP_IPV6_NODE, &neighbor_send_community_cmd);
  install_element (BGP_IPV6_NODE, &neighbor_send_community_type_cmd);
  install_element (BGP_IPV6_NODE, &no_neighbor_send_community_cmd);
  install_element (BGP_IPV6_NODE, &no_neighbor_send_community_type_cmd);

  /* "neighbor weight" commands. */
  install_element (BGP_NODE, &neighbor_weight_cmd);
  install_element (BGP_NODE, &no_neighbor_weight_cmd);
  install_element (BGP_NODE, &no_neighbor_weight_val_cmd);

  /* "neighbor softreconfiguration inbound" commands.*/
  install_element (BGP_NODE, &neighbor_soft_reconfiguration_cmd);
  install_element (BGP_NODE, &no_neighbor_soft_reconfiguration_cmd);
  install_element (BGP_IPV4M_NODE, &neighbor_soft_reconfiguration_cmd);
  install_element (BGP_IPV4M_NODE, &no_neighbor_soft_reconfiguration_cmd);
  install_element (BGP_IPV6_NODE, &neighbor_soft_reconfiguration_cmd);
  install_element (BGP_IPV6_NODE, &no_neighbor_soft_reconfiguration_cmd);

  /* "neighbor route-reflector" commands.*/
  install_element (BGP_NODE, &neighbor_route_reflector_client_cmd);
  install_element (BGP_NODE, &no_neighbor_route_reflector_client_cmd);
  install_element (BGP_IPV4M_NODE, &neighbor_route_reflector_client_cmd);
  install_element (BGP_IPV4M_NODE, &no_neighbor_route_reflector_client_cmd);
  install_element (BGP_IPV6_NODE, &neighbor_route_reflector_client_cmd);
  install_element (BGP_IPV6_NODE, &no_neighbor_route_reflector_client_cmd);

  /* "neighbor route-server" commands.*/
  install_element (BGP_NODE, &neighbor_route_server_client_cmd);
  install_element (BGP_NODE, &no_neighbor_route_server_client_cmd);
  install_element (BGP_IPV4M_NODE, &neighbor_route_server_client_cmd);
  install_element (BGP_IPV4M_NODE, &no_neighbor_route_server_client_cmd);
  install_element (BGP_IPV6_NODE, &neighbor_route_server_client_cmd);
  install_element (BGP_IPV6_NODE, &no_neighbor_route_server_client_cmd);

  /* "neighbor capability route-refresh" commands.*/
  install_element (BGP_NODE, &neighbor_capability_route_refresh_cmd);
  install_element (BGP_NODE, &no_neighbor_capability_route_refresh_cmd);

  /* "neighbor translate-update" commands. */
  install_element (BGP_NODE, &neighbor_translate_update_multicast_cmd);
  install_element (BGP_NODE, &neighbor_translate_update_unimulti_cmd);
  install_element (BGP_NODE, &no_neighbor_translate_update_cmd);
  install_element (BGP_NODE, &no_neighbor_translate_update_multicast_cmd);
  install_element (BGP_NODE, &no_neighbor_translate_update_unimulti_cmd);

  /* "neighbor dont-capability-negotiate" commands. */
  install_element (BGP_NODE, &neighbor_dont_capability_negotiate_cmd);
  install_element (BGP_NODE, &no_neighbor_dont_capability_negotiate_cmd);

  /* "neighbor override-capability" commands. */
  install_element (BGP_NODE, &neighbor_override_capability_cmd);
  install_element (BGP_NODE, &no_neighbor_override_capability_cmd);

  /* "neighbor strict-capability-match" commands. */
  install_element (BGP_NODE, &neighbor_strict_capability_cmd);
  install_element (BGP_NODE, &no_neighbor_strict_capability_cmd);

  /* "neighbor timers" commands. */
  install_element (BGP_NODE, &neighbor_timers_cmd);
  install_element (BGP_NODE, &no_neighbor_timers_cmd);

  /* "neighbor timers connect" commands. */
  install_element (BGP_NODE, &neighbor_timers_connect_cmd);
  install_element (BGP_NODE, &no_neighbor_timers_connect_cmd);

  /* Filters */
  install_element (BGP_NODE, &neighbor_distribute_list_cmd);
  install_element (BGP_NODE, &no_neighbor_distribute_list_cmd);
  install_element (BGP_NODE, &neighbor_prefix_list_cmd);
  install_element (BGP_NODE, &no_neighbor_prefix_list_cmd);
  install_element (BGP_NODE, &neighbor_filter_list_cmd);
  install_element (BGP_NODE, &no_neighbor_filter_list_cmd);
  install_element (BGP_NODE, &neighbor_route_map_cmd);
  install_element (BGP_NODE, &no_neighbor_route_map_cmd);
  install_element (BGP_IPV4M_NODE, &neighbor_distribute_list_cmd);
  install_element (BGP_IPV4M_NODE, &no_neighbor_distribute_list_cmd);
  install_element (BGP_IPV4M_NODE, &neighbor_prefix_list_cmd);
  install_element (BGP_IPV4M_NODE, &no_neighbor_prefix_list_cmd);
  install_element (BGP_IPV4M_NODE, &neighbor_filter_list_cmd);
  install_element (BGP_IPV4M_NODE, &no_neighbor_filter_list_cmd);
  install_element (BGP_IPV4M_NODE, &neighbor_route_map_cmd);
  install_element (BGP_IPV4M_NODE, &no_neighbor_route_map_cmd);
#if 0
  install_element (BGP_NODE, &neighbor_peer_group_cmd);
  install_element (BGP_NODE, &neighbor_peer_group_remote_as_cmd);
#endif /* 0 */

  /* "neighbor maximum-prefix" commands. */
  install_element (BGP_NODE, &neighbor_maximum_prefix_cmd);
  install_element (BGP_NODE, &neighbor_maximum_prefix_warning_cmd);
  install_element (BGP_NODE, &no_neighbor_maximum_prefix_cmd);
  install_element (BGP_NODE, &no_neighbor_maximum_prefix_val_cmd);
  install_element (BGP_NODE, &no_neighbor_maximum_prefix_val2_cmd);
  install_element (BGP_IPV4M_NODE, &neighbor_maximum_prefix_cmd);
  install_element (BGP_IPV4M_NODE, &neighbor_maximum_prefix_warning_cmd);
  install_element (BGP_IPV4M_NODE, &no_neighbor_maximum_prefix_cmd);
  install_element (BGP_IPV4M_NODE, &no_neighbor_maximum_prefix_val_cmd);
  install_element (BGP_IPV4M_NODE, &no_neighbor_maximum_prefix_val2_cmd);
  install_element (BGP_IPV6_NODE, &neighbor_maximum_prefix_cmd);
  install_element (BGP_IPV6_NODE, &neighbor_maximum_prefix_warning_cmd);
  install_element (BGP_IPV6_NODE, &no_neighbor_maximum_prefix_cmd);
  install_element (BGP_IPV6_NODE, &no_neighbor_maximum_prefix_val_cmd);
  install_element (BGP_IPV6_NODE, &no_neighbor_maximum_prefix_val2_cmd);

  /* "bgp confederation" commands. */
  install_element (BGP_NODE, &bgp_confederation_identifier_cmd);
  install_element (BGP_NODE, &bgp_confederation_peers_cmd);
  install_element (BGP_NODE, &no_bgp_confederation_identifier_cmd);
  install_element (BGP_NODE, &no_bgp_confederation_peers_cmd);

  /* "transparent-as" commands. */
  install_element (BGP_NODE, &neighbor_transparent_as_cmd);
  install_element (BGP_NODE, &no_neighbor_transparent_as_cmd);

  /* "transparent-nexthop" commands. */
  install_element (BGP_NODE, &neighbor_transparent_nexthop_cmd);
  install_element (BGP_NODE, &no_neighbor_transparent_nexthop_cmd);

  /* "show ip bgp summary" commands. */
  install_element (VIEW_NODE, &show_ip_bgp_summary_cmd);
  install_element (VIEW_NODE, &show_ip_bgp_ipv4_summary_cmd);
  install_element (VIEW_NODE, &show_ip_bgp_vpnv4_all_summary_cmd);
  install_element (VIEW_NODE, &show_ip_bgp_vpnv4_rd_summary_cmd);

  install_element (ENABLE_NODE, &show_ip_bgp_summary_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_ipv4_summary_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_vpnv4_all_summary_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_vpnv4_rd_summary_cmd);

  /* "show ip bgp neighbors" commands. */
  install_element (VIEW_NODE, &show_ip_bgp_neighbors_cmd);
  install_element (VIEW_NODE, &show_ip_bgp_ipv4_neighbors_cmd);
  install_element (VIEW_NODE, &show_ip_bgp_neighbors_peer_cmd);
  install_element (VIEW_NODE, &show_ip_bgp_ipv4_neighbors_peer_cmd);
  install_element (VIEW_NODE, &show_ip_bgp_vpnv4_all_neighbors_cmd);
  install_element (VIEW_NODE, &show_ip_bgp_vpnv4_rd_neighbors_cmd);
  install_element (VIEW_NODE, &show_ip_bgp_vpnv4_all_neighbors_peer_cmd);
  install_element (VIEW_NODE, &show_ip_bgp_vpnv4_rd_neighbors_peer_cmd);

  install_element (ENABLE_NODE, &show_ip_bgp_neighbors_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_ipv4_neighbors_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_neighbors_peer_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_ipv4_neighbors_peer_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_vpnv4_all_neighbors_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_vpnv4_rd_neighbors_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_vpnv4_all_neighbors_peer_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_vpnv4_rd_neighbors_peer_cmd);

  /* "show ip bgp paths" commands. */
  install_element (VIEW_NODE, &show_ip_bgp_paths_cmd);
  install_element (VIEW_NODE, &show_ip_bgp_ipv4_paths_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_paths_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_ipv4_paths_cmd);

  /* "show ip bgp community" commands. */
  install_element (VIEW_NODE, &show_ip_bgp_community_info_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_community_info_cmd);

  /* "show ip bgp attribute-info" commands. */
  install_element (VIEW_NODE, &show_ip_bgp_attr_info_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_attr_info_cmd);

  /* "clear ip bgp commands" */
  install_element (ENABLE_NODE, &clear_ip_bgp_all_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_as_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_peer_cmd);
#if 0 
  install_element (ENABLE_NODE, &clear_ip_bgp_peer_group_cmd);
#endif /* 0 */

  /* "clear ip bgp neighbor soft in" */
  install_element (ENABLE_NODE, &clear_ip_bgp_peer_soft_in_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_peer_in_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_peer_ipv4_soft_in_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_peer_ipv4_in_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_as_soft_in_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_as_in_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_as_ipv4_soft_in_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_as_ipv4_in_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_all_soft_in_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_all_in_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_all_ipv4_soft_in_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_all_ipv4_in_cmd);

  /* "clear ip bgp neighbor soft out" */
  install_element (ENABLE_NODE, &clear_ip_bgp_peer_soft_out_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_peer_out_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_peer_ipv4_soft_out_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_peer_ipv4_out_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_as_soft_out_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_as_out_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_as_ipv4_soft_out_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_as_ipv4_out_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_all_soft_out_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_all_out_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_all_ipv4_soft_out_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_all_ipv4_out_cmd);

  /* "clear ip bgp neighbor soft" */
  install_element (ENABLE_NODE, &clear_ip_bgp_peer_soft_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_peer_ipv4_soft_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_as_soft_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_as_ipv4_soft_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_all_soft_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_all_ipv4_soft_cmd);


  /* "clear ip bgp vpnv4 soft in" */
  install_element (ENABLE_NODE, &clear_ip_bgp_peer_vpnv4_soft_in_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_peer_vpnv4_in_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_as_vpnv4_soft_in_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_as_vpnv4_in_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_all_vpnv4_soft_in_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_all_vpnv4_in_cmd);

  /* "clear ip bgp vpnv4 soft out" */
  install_element (ENABLE_NODE, &clear_ip_bgp_peer_vpnv4_soft_out_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_peer_vpnv4_out_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_as_vpnv4_soft_out_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_as_vpnv4_out_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_all_vpnv4_soft_out_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_all_vpnv4_out_cmd);

  /* "clear ip bgp vpnv4 soft" */
  install_element (ENABLE_NODE, &clear_ip_bgp_peer_vpnv4_soft_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_as_vpnv4_soft_cmd);
  install_element (ENABLE_NODE, &clear_ip_bgp_all_vpnv4_soft_cmd);

#ifdef HAVE_IPV6
  /* BGP-MP for IPv6 unicast.  */
  install_element (BGP_NODE, &address_family_ipv6_cmd);
  install_element (BGP_NODE, &address_family_ipv6_unicast_cmd);
  install_element (BGP_IPV6_NODE, &exit_address_family_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_remote_as_cmd);
  install_element (BGP_NODE, &ipv6_bgp_neighbor_remote_as_unicast_cmd);
  install_element (BGP_NODE, &ipv6_bgp_neighbor_remote_as_multicast_cmd);
  install_element (BGP_NODE, &ipv6_bgp_neighbor_remote_as_unicast_multicast_cmd);

  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_remote_as_cmd);

  /* "IPv6 neighbor passive" commands. */
  install_element (BGP_NODE, &ipv6_bgp_neighbor_passive_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_passive_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_shutdown_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_shutdown_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_ebgp_multihop_cmd);
  install_element (BGP_NODE, &ipv6_bgp_neighbor_ebgp_multihop_ttl_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_ebgp_multihop_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_ebgp_multihop_ttl_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_description_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_description_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_description_val_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_version_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_version_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_interface_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_interface_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_nexthop_self_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_nexthop_self_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_update_source_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_update_source_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_default_originate_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_default_originate_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_port_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_port_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_port_val_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_send_community_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_send_community_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_send_community_extended_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_send_community_extended_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_weight_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_weight_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_weight_val_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_soft_reconfiguration_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_soft_reconfiguration_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_route_reflector_client_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_route_reflector_client_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_route_server_client_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_route_server_client_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_capability_route_refresh_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_capability_route_refresh_cmd);

  install_element (BGP_NODE, &ipv6_neighbor_dont_capability_negotiate_cmd);
  install_element (BGP_NODE, &no_ipv6_neighbor_dont_capability_negotiate_cmd);

  install_element (BGP_NODE, &ipv6_neighbor_override_capability_cmd);
  install_element (BGP_NODE, &no_ipv6_neighbor_override_capability_cmd);

  install_element (BGP_NODE, &ipv6_neighbor_strict_capability_cmd);
  install_element (BGP_NODE, &no_ipv6_neighbor_strict_capability_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_timers_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_timers_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_timers_connect_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_timers_connect_cmd);

  install_element (BGP_IPV6_NODE, &ipv6_bgp_neighbor_distribute_list_cmd);
  install_element (BGP_IPV6_NODE, &no_ipv6_bgp_neighbor_distribute_list_cmd);
  install_element (BGP_IPV6_NODE, &ipv6_bgp_neighbor_prefix_list_cmd);
  install_element (BGP_IPV6_NODE, &no_ipv6_bgp_neighbor_prefix_list_cmd);
  install_element (BGP_IPV6_NODE, &ipv6_bgp_neighbor_filter_list_cmd);
  install_element (BGP_IPV6_NODE, &no_ipv6_bgp_neighbor_filter_list_cmd);
  install_element (BGP_IPV6_NODE, &ipv6_bgp_neighbor_route_map_cmd);
  install_element (BGP_IPV6_NODE, &no_ipv6_bgp_neighbor_route_map_cmd);

  install_element (BGP_NODE, &old_ipv6_bgp_neighbor_distribute_list_cmd);
  install_element (BGP_NODE, &old_no_ipv6_bgp_neighbor_distribute_list_cmd);
  install_element (BGP_NODE, &old_ipv6_bgp_neighbor_prefix_list_cmd);
  install_element (BGP_NODE, &old_no_ipv6_bgp_neighbor_prefix_list_cmd);
  install_element (BGP_NODE, &old_ipv6_bgp_neighbor_filter_list_cmd);
  install_element (BGP_NODE, &old_no_ipv6_bgp_neighbor_filter_list_cmd);
  install_element (BGP_NODE, &old_ipv6_bgp_neighbor_route_map_cmd);
  install_element (BGP_NODE, &old_no_ipv6_bgp_neighbor_route_map_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_transparent_as_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_transparent_as_cmd);

  install_element (BGP_NODE, &ipv6_bgp_neighbor_transparent_nexthop_cmd);
  install_element (BGP_NODE, &no_ipv6_bgp_neighbor_transparent_nexthop_cmd);

  install_element (VIEW_NODE, &show_bgp_summary_cmd);
  install_element (VIEW_NODE, &show_bgp_ipv6_summary_cmd);
  install_element (VIEW_NODE, &show_bgp_neighbors_cmd);
  install_element (VIEW_NODE, &show_bgp_ipv6_neighbors_cmd);
  install_element (VIEW_NODE, &show_bgp_neighbors_peer_cmd);
  install_element (VIEW_NODE, &show_bgp_ipv6_neighbors_peer_cmd);

  install_element (ENABLE_NODE, &show_bgp_summary_cmd);
  install_element (ENABLE_NODE, &show_bgp_ipv6_summary_cmd);
  install_element (ENABLE_NODE, &show_bgp_neighbors_cmd);
  install_element (ENABLE_NODE, &show_bgp_ipv6_neighbors_cmd);
  install_element (ENABLE_NODE, &show_bgp_neighbors_peer_cmd);
  install_element (ENABLE_NODE, &show_bgp_ipv6_neighbors_peer_cmd);

/* old commad */
  install_element (VIEW_NODE, &show_ipv6_bgp_summary_cmd);
  install_element (VIEW_NODE, &show_ipv6_mbgp_summary_cmd);

/* old commad */
  install_element (ENABLE_NODE, &show_ipv6_bgp_summary_cmd);
  install_element (ENABLE_NODE, &show_ipv6_mbgp_summary_cmd);

/* old commad */
  install_element (VIEW_NODE, &show_ipv6_bgp_neighbors_cmd);
  install_element (VIEW_NODE, &show_ipv6_bgp_neighbors_peer_cmd);
  install_element (VIEW_NODE, &show_ipv6_mbgp_neighbors_cmd);
  install_element (VIEW_NODE, &show_ipv6_mbgp_neighbors_peer_cmd);

/* old commad */
  install_element (ENABLE_NODE, &show_ipv6_bgp_neighbors_cmd);
  install_element (ENABLE_NODE, &show_ipv6_bgp_neighbors_peer_cmd);
  install_element (ENABLE_NODE, &show_ipv6_mbgp_neighbors_cmd);
  install_element (ENABLE_NODE, &show_ipv6_mbgp_neighbors_peer_cmd);

  install_element (ENABLE_NODE, &clear_bgp_all_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_all_cmd);
  install_element (ENABLE_NODE, &clear_bgp_peer_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_peer_cmd);
#if 0
  install_element (ENABLE_NODE, &clear_bgp_peer_group_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_peer_group_cmd);
#endif /* 0 */
  install_element (ENABLE_NODE, &clear_bgp_as_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_as_cmd);

  install_element (ENABLE_NODE, &clear_bgp_peer_soft_in_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_peer_soft_in_cmd);
  install_element (ENABLE_NODE, &clear_bgp_peer_in_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_peer_in_cmd);
  install_element (ENABLE_NODE, &clear_bgp_as_soft_in_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_as_soft_in_cmd);
  install_element (ENABLE_NODE, &clear_bgp_as_in_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_as_in_cmd);
  install_element (ENABLE_NODE, &clear_bgp_all_soft_in_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_all_soft_in_cmd);
  install_element (ENABLE_NODE, &clear_bgp_all_in_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_all_in_cmd);

  install_element (ENABLE_NODE, &clear_bgp_peer_soft_out_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_peer_soft_out_cmd);
  install_element (ENABLE_NODE, &clear_bgp_peer_out_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_peer_out_cmd);
  install_element (ENABLE_NODE, &clear_bgp_as_soft_out_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_as_soft_out_cmd);
  install_element (ENABLE_NODE, &clear_bgp_as_out_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_as_out_cmd);
  install_element (ENABLE_NODE, &clear_bgp_all_soft_out_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_all_soft_out_cmd);
  install_element (ENABLE_NODE, &clear_bgp_all_out_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_all_out_cmd);

  install_element (ENABLE_NODE, &clear_bgp_peer_soft_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_peer_soft_cmd);
  install_element (ENABLE_NODE, &clear_bgp_as_soft_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_as_soft_cmd);
  install_element (ENABLE_NODE, &clear_bgp_all_soft_cmd);
  install_element (ENABLE_NODE, &clear_bgp_ipv6_all_soft_cmd);
#endif /* HAVE_IPV6 */

  /* BGP-MP for IPv4 multicast.  */
  install_element (BGP_NODE, &address_family_ipv4_multicast_cmd);
  install_element (BGP_IPV4M_NODE, &exit_address_family_cmd);

  /* Make global lists.  */
  bgp_list = list_new ();
  peer_list = list_new ();
  peer_list->cmp = (int (*)(void *, void *)) peer_list_cmp;

  /* BGP multiple instance. */
  bgp_multiple_instance = 0;

  /* Init zebra. */
  zebra_init ();

  /* BGP inits. */
  bgp_attr_init ();
  bgp_debug_init ();
  bgp_dump_init ();
  bgp_route_init ();
  bgp_route_map_init ();
  bgp_scan_init ();

  /* Access list initialize. */
  access_list_init ();
  access_list_add_hook (bgp_distribute_update);
  access_list_delete_hook (bgp_distribute_update);

  /* Filter list initialize. */
  bgp_filter_init ();
  as_list_add_hook (bgp_aslist_update);
  as_list_delete_hook (bgp_aslist_update);

  /* Prefix list initialize.*/
  prefix_list_init ();
  prefix_list_add_hook (bgp_prefix_list_update);
  prefix_list_delete_hook (bgp_prefix_list_update);

  /* Community list initialize. */
  community_list_init ();

#ifdef HAVE_SNMP
  bgp_snmp_init ();
#endif /* HAVE_SNMP */

  bgp_damp_disable (NULL);
}
