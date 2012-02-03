/*****************************************************************************
  Copyright (c) 2006 EMC Corporation.
  Copyright (c) 2011 Factor-SPE

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.

  Authors: Srinivas Aji <Aji_Srinivas@emc.com>
  Authors: Vitalii Demianets <vitas@nppfactor.kiev.ua>

******************************************************************************/

#ifndef CTL_SOCKET_H
#define CTL_SOCKET_H

#include <linux/if_bridge.h>
#include <asm/byteorder.h>

#include "mstp.h"

struct ctl_msg_hdr
{
    int cmd;
    int lin;
    int lout;
    int llog;
    int res;
};

#define LOG_STRING_LEN 256

typedef struct _log_string
{
    char buf[LOG_STRING_LEN];
} LogString;

#define set_socket_address(sa, string) do{  \
    struct sockaddr_un * tmp_sa = (sa);     \
    memset(tmp_sa, 0, sizeof(*tmp_sa));     \
    tmp_sa->sun_family = AF_UNIX;           \
    strcpy(tmp_sa->sun_path + 1, (string)); \
  }while(0)

#define MSTP_SERVER_SOCK_NAME ".mstp_server"

/* Commands sent from bridge-stp script need this flag */
#define RESPONSE_FIRST_HANDLE_LATER     0x10000

/* COMMANDS */
#define CTL_DECLARE(name) \
int CTL_ ## name name ## _ARGS

/* get_cist_bridge_status */
#define CMD_CODE_get_cist_bridge_status 101
#define get_cist_bridge_status_ARGS (int br_index, CIST_BridgeStatus *status, \
                                     char *root_port_name)
struct get_cist_bridge_status_IN
{
    int br_index;
};
struct get_cist_bridge_status_OUT
{
    CIST_BridgeStatus status;
    char root_port_name[IFNAMSIZ];
};
#define get_cist_bridge_status_COPY_IN  ({ in->br_index = br_index; })
#define get_cist_bridge_status_COPY_OUT ({ *status = out->status; \
    strncpy(root_port_name, out->root_port_name, IFNAMSIZ); })
#define get_cist_bridge_status_CALL (in->br_index, &out->status, \
                                     out->root_port_name)
CTL_DECLARE(get_cist_bridge_status);

/* get_msti_bridge_status */
#define CMD_CODE_get_msti_bridge_status 102
#define get_msti_bridge_status_ARGS (int br_index, __u16 mstid, \
                                     MSTI_BridgeStatus *status, \
                                     char *root_port_name)
struct get_msti_bridge_status_IN
{
    int br_index;
    __u16 mstid;
};
struct get_msti_bridge_status_OUT
{
    MSTI_BridgeStatus status;
    char root_port_name[IFNAMSIZ];
};
#define get_msti_bridge_status_COPY_IN \
    ({ in->br_index = br_index; in->mstid = mstid; })
#define get_msti_bridge_status_COPY_OUT ({ *status = out->status; \
    strncpy(root_port_name, out->root_port_name, IFNAMSIZ); })
#define get_msti_bridge_status_CALL (in->br_index, in->mstid, &out->status, \
                                     out->root_port_name)
CTL_DECLARE(get_msti_bridge_status);

/* set_cist_bridge_config */
#define CMD_CODE_set_cist_bridge_config 103
#define set_cist_bridge_config_ARGS (int br_index, CIST_BridgeConfig *cfg)
struct set_cist_bridge_config_IN
{
    int br_index;
    CIST_BridgeConfig cfg;
};
struct set_cist_bridge_config_OUT
{
};
#define set_cist_bridge_config_COPY_IN \
    ({ in->br_index = br_index; in->cfg = *cfg; })
#define set_cist_bridge_config_COPY_OUT ({ (void)0; })
#define set_cist_bridge_config_CALL (in->br_index, &in->cfg)
CTL_DECLARE(set_cist_bridge_config);

/* set_msti_bridge_config */
#define CMD_CODE_set_msti_bridge_config 104
#define set_msti_bridge_config_ARGS (int br_index, __u16 mstid, \
                                     __u8 bridge_priority)
struct set_msti_bridge_config_IN
{
    int br_index;
    __u16 mstid;
    __u8 bridge_priority;
};
struct set_msti_bridge_config_OUT
{
};
#define set_msti_bridge_config_COPY_IN             \
    ({ in->br_index = br_index; in->mstid = mstid; \
       in->bridge_priority = bridge_priority; })
#define set_msti_bridge_config_COPY_OUT ({ (void)0; })
#define set_msti_bridge_config_CALL (in->br_index, in->mstid, \
                                     in->bridge_priority)
CTL_DECLARE(set_msti_bridge_config);

/* get_cist_port_status */
#define CMD_CODE_get_cist_port_status   105
#define get_cist_port_status_ARGS (int br_index, int port_index, \
                                   CIST_PortStatus *status)
struct get_cist_port_status_IN
{
    int br_index;
    int port_index;
};
struct get_cist_port_status_OUT
{
    CIST_PortStatus status;
};
#define get_cist_port_status_COPY_IN \
    ({ in->br_index = br_index; in->port_index = port_index; })
#define get_cist_port_status_COPY_OUT ({ *status = out->status; })
#define get_cist_port_status_CALL (in->br_index, in->port_index, &out->status)
CTL_DECLARE(get_cist_port_status);

/* get_msti_port_status */
#define CMD_CODE_get_msti_port_status   106
#define get_msti_port_status_ARGS (int br_index, int port_index, __u16 mstid, \
                                   MSTI_PortStatus *status)
struct get_msti_port_status_IN
{
    int br_index;
    int port_index;
    __u16 mstid;
};
struct get_msti_port_status_OUT
{
    MSTI_PortStatus status;
};
#define get_msti_port_status_COPY_IN \
    ({ in->br_index = br_index; in->port_index = port_index; \
       in->mstid = mstid; })
#define get_msti_port_status_COPY_OUT ({ *status = out->status; })
#define get_msti_port_status_CALL (in->br_index, in->port_index, in->mstid, \
                                   &out->status)
CTL_DECLARE(get_msti_port_status);

/* set_cist_port_config */
#define CMD_CODE_set_cist_port_config   107
#define set_cist_port_config_ARGS (int br_index, int port_index, \
                                   CIST_PortConfig *cfg)
struct set_cist_port_config_IN
{
    int br_index;
    int port_index;
    CIST_PortConfig cfg;
};
struct set_cist_port_config_OUT
{
};
#define set_cist_port_config_COPY_IN \
    ({ in->br_index = br_index; in->port_index = port_index; in->cfg = *cfg; })
#define set_cist_port_config_COPY_OUT ({ (void)0; })
#define set_cist_port_config_CALL (in->br_index, in->port_index, &in->cfg)
CTL_DECLARE(set_cist_port_config);

/* set_msti_port_config */
#define CMD_CODE_set_msti_port_config   108
#define set_msti_port_config_ARGS (int br_index, int port_index, __u16 mstid, \
                                   MSTI_PortConfig *cfg)
struct set_msti_port_config_IN
{
    int br_index;
    int port_index;
    __u16 mstid;
    MSTI_PortConfig cfg;
};
struct set_msti_port_config_OUT
{
};
#define set_msti_port_config_COPY_IN \
    ({ in->br_index = br_index; in->port_index = port_index; \
       in->mstid = mstid; in->cfg = *cfg; })
#define set_msti_port_config_COPY_OUT ({ (void)0; })
#define set_msti_port_config_CALL (in->br_index, in->port_index, in->mstid, \
                                   &in->cfg)
CTL_DECLARE(set_msti_port_config);

/* port_mcheck */
#define CMD_CODE_port_mcheck    109
#define port_mcheck_ARGS (int br_index, int port_index)
struct port_mcheck_IN
{
    int br_index;
    int port_index;
};
struct port_mcheck_OUT
{
};
#define port_mcheck_COPY_IN \
    ({ in->br_index = br_index; in->port_index = port_index; })
#define port_mcheck_COPY_OUT ({ (void)0; })
#define port_mcheck_CALL (in->br_index, in->port_index)
CTL_DECLARE(port_mcheck);

/* set_debug_level */
#define CMD_CODE_set_debug_level    110
#define set_debug_level_ARGS (int level)
struct set_debug_level_IN
{
    int level;
};
struct set_debug_level_OUT
{
};
#define set_debug_level_COPY_IN  ({ in->level = level; })
#define set_debug_level_COPY_OUT ({ (void)0; })
#define set_debug_level_CALL (in->level)
CTL_DECLARE(set_debug_level);

/* get_mstilist */
#define CMD_CODE_get_mstilist   111
#define get_mstilist_ARGS (int br_index, int *num_mstis, __u16 *mstids)
struct get_mstilist_IN
{
    int br_index;
};
struct get_mstilist_OUT
{
    int num_mstis;
    __u16 mstids[MAX_IMPLEMENTATION_MSTIS + 1]; /* +1 - for the CIST */
};
#define get_mstilist_COPY_IN \
    ({ in->br_index = br_index; })
#define get_mstilist_COPY_OUT ({ *num_mstis = out->num_mstis; \
    memcpy(mstids, out->mstids, (*num_mstis) * sizeof(out->mstids[0])); })
#define get_mstilist_CALL (in->br_index, &out->num_mstis, out->mstids)
CTL_DECLARE(get_mstilist);

/* create_msti */
#define CMD_CODE_create_msti    112
#define create_msti_ARGS (int br_index, __u16 mstid)
struct create_msti_IN
{
    int br_index;
    __u16 mstid;
};
struct create_msti_OUT
{
};
#define create_msti_COPY_IN \
    ({ in->br_index = br_index; in->mstid = mstid; })
#define create_msti_COPY_OUT ({ (void)0; })
#define create_msti_CALL (in->br_index, in->mstid)
CTL_DECLARE(create_msti);

/* delete_msti */
#define CMD_CODE_delete_msti    113
#define delete_msti_ARGS (int br_index, __u16 mstid)
struct delete_msti_IN
{
    int br_index;
    __u16 mstid;
};
struct delete_msti_OUT
{
};
#define delete_msti_COPY_IN \
    ({ in->br_index = br_index; in->mstid = mstid; })
#define delete_msti_COPY_OUT ({ (void)0; })
#define delete_msti_CALL (in->br_index, in->mstid)
CTL_DECLARE(delete_msti);

/* get_mstconfid */
#define CMD_CODE_get_mstconfid  114
#define get_mstconfid_ARGS (int br_index, mst_configuration_identifier_t *cfg)
struct get_mstconfid_IN
{
    int br_index;
};
struct get_mstconfid_OUT
{
    mst_configuration_identifier_t cfg;
};
#define get_mstconfid_COPY_IN  ({ in->br_index = br_index; })
#define get_mstconfid_COPY_OUT ({ *cfg = out->cfg; })
#define get_mstconfid_CALL (in->br_index, &out->cfg)
CTL_DECLARE(get_mstconfid);

/* set_mstconfid */
#define CMD_CODE_set_mstconfid  115
#define set_mstconfid_ARGS (int br_index, __u16 revision, char *name)
struct set_mstconfid_IN
{
    int br_index;
    __u16 revision;
    __u8 name[CONFIGURATION_NAME_LEN];
};
struct set_mstconfid_OUT
{
};
#define set_mstconfid_COPY_IN  ({ in->br_index = br_index; \
    in->revision = revision; strncpy(in->name, name, sizeof(in->name)); })
#define set_mstconfid_COPY_OUT ({ (void)0; })
#define set_mstconfid_CALL (in->br_index, in->revision, in->name)
CTL_DECLARE(set_mstconfid);

/* get_vids2fids */
#define CMD_CODE_get_vids2fids  116
#define get_vids2fids_ARGS (int br_index, __u16 *vids2fids)
struct get_vids2fids_IN
{
    int br_index;
};
struct get_vids2fids_OUT
{
    __u16 vids2fids[MAX_VID + 1];
};
#define get_vids2fids_COPY_IN  ({ in->br_index = br_index; })
#define get_vids2fids_COPY_OUT ({ \
    memcpy(vids2fids, out->vids2fids, sizeof(out->vids2fids)); })
#define get_vids2fids_CALL (in->br_index, out->vids2fids)
CTL_DECLARE(get_vids2fids);

/* get_fids2mstids */
#define CMD_CODE_get_fids2mstids    117
#define get_fids2mstids_ARGS (int br_index, __u16 *fids2mstids)
struct get_fids2mstids_IN
{
    int br_index;
};
struct get_fids2mstids_OUT
{
    __u16 fids2mstids[MAX_FID + 1];
};
#define get_fids2mstids_COPY_IN  ({ in->br_index = br_index; })
#define get_fids2mstids_COPY_OUT ({ \
    memcpy(fids2mstids, out->fids2mstids, sizeof(out->fids2mstids)); })
#define get_fids2mstids_CALL (in->br_index, out->fids2mstids)
CTL_DECLARE(get_fids2mstids);

/* set_vid2fid */
#define CMD_CODE_set_vid2fid    118
#define set_vid2fid_ARGS (int br_index, __u16 vid, __u16 fid)
struct set_vid2fid_IN
{
    int br_index;
    __u16 vid, fid;
};
struct set_vid2fid_OUT
{
};
#define set_vid2fid_COPY_IN  ({ in->br_index = br_index; in->vid = vid; \
                                in->fid = fid; })
#define set_vid2fid_COPY_OUT ({ (void)0; })
#define set_vid2fid_CALL (in->br_index, in->vid, in->fid)
CTL_DECLARE(set_vid2fid);

/* set_fid2mstid */
#define CMD_CODE_set_fid2mstid  119
#define set_fid2mstid_ARGS (int br_index, __u16 fid, __u16 mstid)
struct set_fid2mstid_IN
{
    int br_index;
    __u16 fid, mstid;
};
struct set_fid2mstid_OUT
{
};
#define set_fid2mstid_COPY_IN  ({ in->br_index = br_index; in->fid = fid; \
                                in->mstid = mstid; })
#define set_fid2mstid_COPY_OUT ({ (void)0; })
#define set_fid2mstid_CALL (in->br_index, in->fid, in->mstid)
CTL_DECLARE(set_fid2mstid);

/* set_vids2fids */
#define CMD_CODE_set_vids2fids  120
#define set_vids2fids_ARGS (int br_index, __u16 *vids2fids)
struct set_vids2fids_IN
{
    int br_index;
    __u16 vids2fids[MAX_VID + 1];
};
struct set_vids2fids_OUT
{
};
#define set_vids2fids_COPY_IN  ({ in->br_index = br_index; \
    memcpy(in->vids2fids, vids2fids, sizeof(in->vids2fids)); })
#define set_vids2fids_COPY_OUT ({ (void)0; })
#define set_vids2fids_CALL (in->br_index, in->vids2fids)
CTL_DECLARE(set_vids2fids);

/* set_fids2mstids */
#define CMD_CODE_set_fids2mstids    121
#define set_fids2mstids_ARGS (int br_index, __u16 *fids2mstids)
struct set_fids2mstids_IN
{
    int br_index;
    __u16 fids2mstids[MAX_FID + 1];
};
struct set_fids2mstids_OUT
{
};
#define set_fids2mstids_COPY_IN  ({ in->br_index = br_index; \
    memcpy(in->fids2mstids, fids2mstids, sizeof(in->fids2mstids)); })
#define set_fids2mstids_COPY_OUT ({ (void)0; })
#define set_fids2mstids_CALL (in->br_index, in->fids2mstids)
CTL_DECLARE(set_fids2mstids);

/* add bridges */
#define CMD_CODE_add_bridges    (122 | RESPONSE_FIRST_HANDLE_LATER)
#define add_bridges_ARGS (int *br_array, int* *ifaces_lists)
CTL_DECLARE(add_bridges);

/* delete bridges */
#define CMD_CODE_del_bridges    (123 | RESPONSE_FIRST_HANDLE_LATER)
#define del_bridges_ARGS (int *br_array)
CTL_DECLARE(del_bridges);

/* General case part in ctl command server switch */
#define SERVER_MESSAGE_CASE(name)                            \
    case CMD_CODE_ ## name : do                              \
    {                                                        \
        struct name ## _IN in0, *in = &in0;                  \
        struct name ## _OUT out0, *out = &out0;              \
        if(sizeof(*in) != lin || sizeof(*out) != lout)       \
        {                                                    \
            LOG("Bad sizes lin %d != %zd or lout %d != %zd", \
                lin, sizeof(*in), lout, sizeof(*out));       \
            return -1;                                       \
        }                                                    \
        memcpy(in, inbuf, lin);                              \
        int r = CTL_ ## name name ## _CALL;                  \
        if(r)                                                \
            return r;                                        \
        if(outbuf)                                           \
            memcpy(outbuf, out, lout);                       \
        return r;                                            \
    }while(0)

/* Wraper for the control functions in the control command client */
#define CLIENT_SIDE_FUNCTION(name)                               \
CTL_DECLARE(name)                                                \
{                                                                \
    struct name ## _IN in0, *in = &in0;                          \
    struct name ## _OUT out0, *out = &out0;                      \
    name ## _COPY_IN;                                            \
    int res = 0;                                                 \
    LogString log = { .buf = "" };                               \
    int r = send_ctl_message(CMD_CODE_ ## name, in, sizeof(*in), \
                             out, sizeof(*out), &log, &res);     \
    if(r || res)                                                 \
        LOG("Got return code %d, %d\n%s", r, res, log.buf);      \
    if(r)                                                        \
        return r;                                                \
    if(res)                                                      \
        return res;                                              \
    name ## _COPY_OUT;                                           \
    return 0;                                                    \
}

#endif /* CTL_SOCKET_H */
