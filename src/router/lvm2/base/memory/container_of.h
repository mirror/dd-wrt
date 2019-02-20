// Copyright (C) 2018 Red Hat, Inc. All rights reserved.
// 
// This file is part of LVM2.
//
// This copyrighted material is made available to anyone wishing to use,
// modify, copy, or redistribute it subject to the terms and conditions
// of the GNU Lesser General Public License v.2.1.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#ifndef BASE_MEMORY_CONTAINER_OF_H
#define BASE_MEMORY_CONTAINER_OF_H

//----------------------------------------------------------------

#define container_of(v, t, head) \
    ((t *)((const char *)(v) - (const char *)&((t *) 0)->head))

//----------------------------------------------------------------

#endif
