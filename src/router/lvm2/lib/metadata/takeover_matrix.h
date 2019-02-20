/*
 * Copyright (C) 2016 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#define N _takeover_noop
#define X _takeover_unsupported

#define lin_r0   _takeover_from_linear_to_raid0
#define lin_r0   _takeover_from_linear_to_raid0
#define lin_r1   _takeover_from_linear_to_raid1
#define lin_r10  _takeover_from_linear_to_raid10
#define lin_r45  _takeover_from_linear_to_raid45
#define mir_r0   _takeover_from_mirrored_to_raid0
#define mir_r0m  _takeover_from_mirrored_to_raid0_meta
#define mir_r1   _takeover_from_mirrored_to_raid1
#define mir_r10  _takeover_from_mirrored_to_raid10
#define mir_r45  _takeover_from_mirrored_to_raid45
#define r01_r01  _takeover_from_raid01_to_raid01
#define r01_r10  _takeover_from_raid01_to_raid10
#define r01_str  _takeover_from_raid01_to_striped
#define r0__lin  _takeover_from_raid0_to_linear
#define r0__mir  _takeover_from_raid0_to_mirrored
#define r0m_lin  _takeover_from_raid0_meta_to_linear
#define r0m_mir  _takeover_from_raid0_meta_to_mirrored
#define r0m_r0   _takeover_from_raid0_meta_to_raid0
#define r0m_r1   _takeover_from_raid0_meta_to_raid1
#define r0m_r10  _takeover_from_raid0_meta_to_raid10
#define r0m_r45  _takeover_from_raid0_meta_to_raid45
#define r0m_r6   _takeover_from_raid0_meta_to_raid6
#define r0m_str  _takeover_from_raid0_meta_to_striped
#define r0__r0m  _takeover_from_raid0_to_raid0_meta
#define r0__r1   _takeover_from_raid0_to_raid1
#define r0__r10  _takeover_from_raid0_to_raid10
#define r0__r45  _takeover_from_raid0_to_raid45
#define r0__r6   _takeover_from_raid0_to_raid6
#define r0__str  _takeover_from_raid0_to_striped
#define r10_lin  _takeover_from_raid10_to_linear
#define r10_mir  _takeover_from_raid10_to_mirrored
#define r10_r0   _takeover_from_raid10_to_raid0
#define r10_r01  _takeover_from_raid10_to_raid01
#define r10_r0m  _takeover_from_raid10_to_raid0_meta
#define r10_r1   _takeover_from_raid10_to_raid1
#define r10_r10  _takeover_from_raid10_to_raid10
#define r10_str  _takeover_from_raid10_to_striped
#define r1__lin  _takeover_from_raid1_to_linear
#define r1__mir  _takeover_from_raid1_to_mirrored
#define r1__r0   _takeover_from_raid1_to_raid0
#define r1__r0m  _takeover_from_raid1_to_raid0_meta
#define r1__r1   _takeover_from_raid1_to_raid1
#define r1__r10  _takeover_from_raid1_to_raid10
#define r1__r5   _takeover_from_raid1_to_raid5
#define r1__str  _takeover_from_raid1_to_striped
#define r45_lin  _takeover_from_raid45_to_linear
#define r45_mir  _takeover_from_raid45_to_mirrored
#define r45_r0   _takeover_from_raid45_to_raid0
#define r45_r0m  _takeover_from_raid45_to_raid0_meta
#define r5_r1    _takeover_from_raid5_to_raid1
#define r45_r54  _takeover_from_raid45_to_raid54
#define r45_r6   _takeover_from_raid45_to_raid6
#define r45_str  _takeover_from_raid45_to_striped
#define r6__r0   _takeover_from_raid6_to_raid0
#define r6__r0m  _takeover_from_raid6_to_raid0_meta
#define r6__r45  _takeover_from_raid6_to_raid45
#define r6__str  _takeover_from_raid6_to_striped
#define str_r0   _takeover_from_striped_to_raid0
#define str_r01  _takeover_from_striped_to_raid01
#define str_r0m  _takeover_from_striped_to_raid0_meta
#define str_r10  _takeover_from_striped_to_raid10
#define str_r45  _takeover_from_striped_to_raid45
#define str_r6   _takeover_from_striped_to_raid6

static uint64_t _segtype_index[] = {
	1, /* linear */
	1, /* striped */
	SEG_MIRROR,
	SEG_RAID0,
	SEG_RAID0_META,
	SEG_RAID1,
	SEG_RAID4 | SEG_RAID5_LS | SEG_RAID5_LA | SEG_RAID5_LS | SEG_RAID5_RS | SEG_RAID5_RA | SEG_RAID5_N,
	SEG_RAID6_LS_6 | SEG_RAID6_LA_6 | SEG_RAID6_RS_6 | SEG_RAID6_RA_6 | SEG_RAID6_NC | SEG_RAID6_NR | SEG_RAID6_ZR | SEG_RAID6_N_6,
	0, // SEG_RAID10_NEAR | SEG_RAID10_FAR | SEG_RAID10_OFFSET,
	0, // SEG_RAID01,
	0
};

/*
 * Matrix of takeover functions.
 * Row corresponds to original segment type.
 * Column corresponds to new segment type.
 * N represents a combination that has no effect (no-op).
 * X represents a combination that is unsupported.
 */
static takeover_fn_t _takeover_fns[][11] = {
        /* from, to ->    linear     striped   mirror    raid0    raid0_meta raid1   raid4/5   raid6    raid10    raid01   other*/
        /*   | */
        /*   v */
        /* linear     */ { N      ,  X      ,  X      ,  lin_r0,  lin_r0 ,  lin_r1,  lin_r45,  X     ,  lin_r10,  X      , X },
        /* striped    */ { X      ,  N      ,  X      ,  str_r0,  str_r0m,  lin_r1,  str_r45,  str_r6,  str_r10,  str_r01, X },
        /* mirror     */ { X      ,  X      ,  N      ,  mir_r0,  mir_r0m,  mir_r1,  mir_r45,  X     ,  mir_r10,  X      , X },
        /* raid0      */ { r0__lin,  r0__str,  r0__mir,  N     ,  r0__r0m,  r0__r1,  r0__r45,  r0__r6,  r0__r10,  X      , X },
        /* raid0_meta */ { r0m_lin,  r0m_str,  r0m_mir,  r0m_r0,  N      ,  r0m_r1,  r0m_r45,  r0m_r6,  r0m_r10,  X      , X },
        /* raid1      */ { r1__lin,  r1__str,  r1__mir,  r1__r0,  r1__r0m,  r1__r1,  r1__r5,   X     ,  r1__r10,  X      , X },
        /* raid4/5    */ { r45_lin,  r45_str,  r45_mir,  r45_r0,  r45_r0m,  r5_r1 ,  r45_r54,  r45_r6,  X      ,  X      , X },
        /* raid6      */ { X      ,  r6__str,  X      ,  r6__r0,  r6__r0m,  X     ,  r6__r45,  X     ,  X      ,  X      , X },
        /* raid10     */ { r10_lin,  r10_str,  r10_mir,  r10_r0,  r10_r0m,  r10_r1,  X      ,  X     ,  X      ,  X	 , X },
        /* raid01     */ // { X      ,  r01_str,  X      ,  X     ,  X      ,  X     ,  X      ,  X     ,  r01_r10,  r01_r01, X },
        /* other      */ { X      ,  X      ,  X      ,  X     ,  X      ,  X     ,  X      ,  X     ,  X      ,  X      , X },
};
#undef X
#undef N
