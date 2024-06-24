/*
 * Copyright (C) 2010, 2011, 2013, 2014 Robert Lougher <rob@jamvm.org.uk>.
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

#define IS_JTHREAD(cb) (cb->flags & JTHREAD)

extern void fillInStackTrace(Object *);
extern Object *consNewInstance(Object *reflect_ob, Object *args_array);
extern Object *invokeMethod(Object *reflect_ob, Object *ob, Object *args_array);

extern int stackTraceDepth(Object *thrwble);
extern Object *stackTraceElementAtIndex(Object *thrwble, int index);

extern int initialiseJVMInterface();
extern Object *enclosingMethodInfo(Class *class);
extern Object *getAnnotationsAsArray(AttributeData *annotations);

extern int typeNo2PrimTypeIndex(int type_no);
extern int primTypeIndex2Size(int prim_idx);
extern char primClass2TypeChar(Class *prim);
extern int widenPrimitiveElement(int src_idx, int dst_idx,
	                         void *src_addr, void *dst_addr);

extern int jThreadIsAlive(Object *jthread);
extern void *getJMMInterface(int version);

extern void initialiseMethodHandles();
extern void expandMemberName(Object *mname);
extern void initMemberName(Object *mname, Object *target);
extern long long memNameFieldOffset(Object *mname);
extern long long memNameStaticFieldOffset(Object *mname);
extern void setCallSiteTargetNormal(Object *call_site, Object *target);
extern void setCallSiteTargetVolatile(Object *call_site, Object *target);
extern int getMembers(Class *clazz, Object *match_name, Object *match_sig,
                      int match_flags, Class *caller, int skip,
                      Object *results);
extern Object *resolveMemberName(Class *mh_class, Object *mname);

extern Object *getMethodParameters(Object *method);

extern Class *findClassFromLoader(char *name, int init, Object *loader,
                                  int throw_error);
