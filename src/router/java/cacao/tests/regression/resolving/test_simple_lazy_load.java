/* regression/resolving/test_simple_lazy_load.java

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

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
*/

public class test_simple_lazy_load extends TestController {

	public static void main(String[] args) {
		new test_simple_lazy_load();
	}

	test_simple_lazy_load() {
		// ***** setup

		TestLoader ld1 = new TestLoader("ld1", this);

		ld1.addClassfile("BarUseFoo", "classes1/BarUseFoo.class");
		ld1.addParentDelegation("java.lang.Object");

		// OpenJDKs reflection API forces eager loading of classes
		if (ClassLibrary.getCurrent() == ClassLibrary.OPEN_JDK) {
			ld1.addClassfile("Foo", "classes1/Foo.class");
			ld1.addParentDelegation("java.lang.String");
		}

		// ***** test

		expectRequest(ld1, "BarUseFoo")
			.expectRequest("java.lang.Object")
			.expectDelegateToSystem()
		.expectDefinition();

		if (ClassLibrary.getCurrent() == ClassLibrary.OPEN_JDK) {
			// constructor of java.lang.Method checks descriptor of idOfFoo
			// this forces loading of Foo and String
			expectRequest(ld1, "java.lang.String")
			.expectDelegateToSystem();

			expectRequest(ld1, "Foo")
			.expectDefinition();
		}

		Class<?> cls = loadClass(ld1, "BarUseFoo");
		checkClassId(cls, "classes1/BarUseFoo");
		expectEnd();

		ld1.addClassfile("Foo", "classes1/Foo.class");
		setReportClassIDs(true);

		if (ClassLibrary.getCurrent() == ClassLibrary.GNU_CLASSPATH) {
			// while OpenJDKs reflection API already forced eager loading of Foo,
			// classpath allows us to do this lazily
			expectRequest(ld1, "Foo")
			.expectDefinitionWithClassId("classes1/Foo");
		}

		checkStringGetter(cls, "idOfFoo", "classes1/Foo");

		exit();
	}
}

/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: java
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */