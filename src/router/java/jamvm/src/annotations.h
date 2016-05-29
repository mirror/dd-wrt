/*
 * Copyright (C) 2013 Robert Lougher <rob@jamvm.org.uk>.
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

#define getClassAnnotationData(class) \
    CLASS_EXTRA_ATTRIBUTES(class, class_annos)

#define getClassTypeAnnotationData(class) \
    CLASS_EXTRA_ATTRIBUTES(class, class_type_annos)

#define getMethodAnnotationData(mb) \
    METHOD_EXTRA_ATTRIBUTES(mb, method_annos)

#define getMethodTypeAnnotationData(mb) \
    METHOD_EXTRA_ATTRIBUTES(mb, method_type_annos)

#define getMethodParameterAnnotationData(mb) \
    METHOD_EXTRA_ATTRIBUTES(mb, method_parameter_annos)

#define getMethodDefaultValueAnnotationData(mb) \
    METHOD_EXTRA_ATTRIBUTES(mb, method_anno_default_val)

#define getFieldAnnotationData(fb) \
    FIELD_EXTRA_ATTRIBUTES(fb, field_annos)

#define getFieldTypeAnnotationData(fb) \
    FIELD_EXTRA_ATTRIBUTES(fb, field_type_annos)
