/* tests/regression/MinimalClassReflection.java - checks most some of the
   reflective methods onto java.lang.Class

   Copyright (C) 1996-2005, 2006 R. Grafl, A. Krall, C. Kruegel,
   C. Oates, R. Obermaisser, M. Platter, M. Probst, S. Ring,
   E. Steiner, C. Thalinger, D. Thuernbeck, P. Tomsich, C. Ullrich,
   TU Wien

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Contact: cacao@cacaojvm.org

   Authors: Mathias Panzenböck

*/

/**
 * Test following methos of java.lang.Class for proper function:
 *  getSuperclass()
 *  getDeclaringClass()
 *  getEnclosingClass()
 *  getEnclosingConstructor()
 *  getEnclosingMethod()
 *
 * I wrote this because getSuperclass() sometimes made problems.
 */
import java.lang.reflect.Method;
import java.lang.reflect.Constructor;

public class MinimalClassReflection {
	public static void printClassInfo(String header, Class<?> cls) {
		Class<?>       clazz;
		Constructor<?> constr;
		Method         method;
		
		p("---------------------- %s ----------------------", header);
		p("is local:              %s", cls.isLocalClass());
		p("is anonymous:          %s", cls.isAnonymousClass());
		p("is member:             %s", cls.isMemberClass());
		p("name:                  %s", cls.getName());
		p("simple name:           %s", cls.getSimpleName());
		p("canonical name:        %s", cls.getCanonicalName());
		
		clazz = cls.getSuperclass();
		p("super class:           %s",
				(clazz == null ? "null" : clazz.getName()));
		
		clazz = cls.getDeclaringClass();
		p("declaring class:       %s",
				(clazz == null ? "null" : clazz.getName()));
		
		clazz = cls.getEnclosingClass();
		p("enclosing class:       %s",
				(clazz == null ? "null" : clazz.getName()));
		
		constr = cls.getEnclosingConstructor();
		p("enclosing constructor: %s",
				(constr == null ? "null" :
					constr.getDeclaringClass().getName() +
					"." + constr.getName()));
		
		method = cls.getEnclosingMethod();
		p("enclosing method:      %s",
				(method == null ? "null" :
					method.getDeclaringClass().getName() +
					"." + method.getName()));
		
		p();
	}
	
	public static void main(String[] args) {
		class ALocalClass {
			class AMemberClass {
			}
			
			public ALocalClass() {
				class AnotherLocalClass {
				}

				printClassInfo(
						"test a member class",
						AMemberClass.class);
				
				printClassInfo(
						"test a anonymous class derived from a member class",
						new AMemberClass() {}.getClass());
				
				printClassInfo(
						"test a local class (local to a constructor)",
						AnotherLocalClass.class);
				
				printClassInfo(
						"test a anonymous class derived from a local " +
						"class (local to a constructor)",
						new AnotherLocalClass() {}.getClass());
			}
		}
		
		printClassInfo(
				"test a normal class",
				MinimalClassReflection.class);
		
		printClassInfo(
				"test a local class (local to a method)",
				ALocalClass.class);
		
		printClassInfo(
				"test a anonymous class",
				new Object() {}.getClass());
		
		printClassInfo(
				"test a anonymous class derived from a local class" +
				" (local to a method)",
				new ALocalClass() {}.getClass());
		
		new ALocalClass();
	}
	
	public static void p(String fmt, Object... args) {
		System.out.printf(fmt + "\n", args);
	}
	
	public static <T> void p(T o) {
		System.out.println(o);
	}
	
	public static void p() {
		System.out.println();
	}
}
