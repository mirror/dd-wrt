/*
 * Copyright (C) 2010, 2012, 2013, 2014 Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "jam.h"
#include "symbol.h"

static Class *magic_accessor;

int classlibInitialiseAccess() {
    magic_accessor = findSystemClass0(SYMBOL(sun_reflect_MagicAccessorImpl));
    if(magic_accessor == NULL)
        return FALSE;

    registerStaticClassRef(&magic_accessor);
    return TRUE;
}

int classlibAccessCheck(Class *class, Class *referrer) {
    Class *host_class = CLASS_CB(referrer)->host_class;

    if(host_class != NULL) {
        while(CLASS_CB(host_class)->host_class != NULL)
            host_class = CLASS_CB(host_class)->host_class;

        if(host_class == class)
            return TRUE;
    }

    return isSubClassOf(magic_accessor, referrer);
}

