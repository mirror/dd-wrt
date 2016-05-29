/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007
 * Robert Lougher <rob@jamvm.org.uk>.
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

char implements(Class *class, Class *test) {
    ClassBlock *test_cb = CLASS_CB(test);
    int i;

    for(i = 0; i < test_cb->interfaces_count; i++)
        if((class == test_cb->interfaces[i]) ||
                      implements(class, test_cb->interfaces[i]))
            return TRUE;

    if(test_cb->super)
        return implements(class, test_cb->super);

    return FALSE;
}

char isSubClassOf(Class *class, Class *test) {
    for(; test != NULL && test != class; test = CLASS_CB(test)->super);
    return test != NULL;
}

char isInstOfArray0(Class *array_class, Class *test_elem, int test_dim) {
    ClassBlock *array_cb = CLASS_CB(array_class);
    Class *array_elem = array_cb->element_class;

    if(test_dim == array_cb->dim)
        return isInstanceOf(array_elem, test_elem);

    if(test_dim > array_cb->dim)
        return IS_INTERFACE(CLASS_CB(array_elem)) ?
                     implements(array_elem, array_class) :
                     (array_elem == array_cb->super);

    return FALSE;
}

char isInstOfArray(Class *class, Class *test) {
    ClassBlock *test_cb = CLASS_CB(test);

    if(!IS_ARRAY(CLASS_CB(class)))
        return class == test_cb->super;

    return isInstOfArray0(class, test_cb->element_class, test_cb->dim);
}

char isInstanceOf(Class *class, Class *test) {
    if(class == test)
        return TRUE;

    if(IS_INTERFACE(CLASS_CB(class)))
        return implements(class, test);
    else
        if(IS_ARRAY(CLASS_CB(test)))
            return isInstOfArray(class, test);       
        else
            return isSubClassOf(class, test); 
}

char arrayStoreCheck(Class *array_class, Class *test) {
    ClassBlock *test_cb = CLASS_CB(test);

    if(!IS_ARRAY(test_cb))
        return isInstOfArray0(array_class, test, 1);
 
    return isInstOfArray0(array_class, test_cb->element_class, test_cb->dim + 1);
}
