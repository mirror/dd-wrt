/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2009-2013 Sourcefire, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef SDF_PATTERN_MATCH__H
#define SDF_PATTERN_MATCH__H

#include "spp_sdf.h"
#include "sdf_detection_option.h"
#include "treenodes.h"
#include <stdint.h>

int AddPii(sdf_tree_node *head, SDFOptionData *data);
int AddPiiPiece(sdf_tree_node *node, char *new_pattern, SDFOptionData *data);
int SplitNode(sdf_tree_node *node, uint16_t split_index);
sdf_tree_node * AddChild(sdf_tree_node *node, SDFOptionData *data, char *pattern);
int FreePiiTree(sdf_tree_node *head);

sdf_tree_node * FindPii(const sdf_tree_node *head, char *buf, uint16_t *buf_index, uint16_t buflen, SDFConfig *config, SDFSessionData *session);
sdf_tree_node * FindPiiRecursively(sdf_tree_node *node, char *buf, uint16_t *buf_index, uint16_t buflen,
                SDFConfig *config, uint16_t *partial_index, sdf_tree_node **partial_node);

#endif /* SDF_PATTERN_MATCH__H */
