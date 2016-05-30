/* regression/resolving/test_return_subtype_ok.java

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

public class test_return_subtype_ok extends TestController {

	public static void main(String[] args) {
		new test_return_subtype_ok();
	}

	test_return_subtype_ok() {
		// ***** setup

		TestLoader ld1 = new TestLoader("ld1", this);
		TestLoader ld2 = new TestLoader("ld2", this);

		ld1.addClassfile("Foo",        "classes1/Foo.class");
		ld1.addClassfile("DerivedFoo", "classes2/DerivedFoo.class");
		ld1.addParentDelegation("java.lang.Object");
		ld1.addParentDelegation("java.lang.String");

		ld2.addClassfile("BarPassFoo", "classes2/BarPassFoo.class");
		ld2.addDelegation("Foo",        ld1);
		ld2.addDelegation("DerivedFoo", ld1);
		ld2.addParentDelegation("java.lang.Object");
		ld2.addParentDelegation("java.lang.String");

		// ***** test

		// loading & linking BarPassFoo
		expectRequest(ld2, "BarPassFoo")
			.expectRequest("java.lang.Object")
			.expectDelegateToSystem()
		.expectDefinition();

		Class<?> cls = loadClass(ld2, "BarPassFoo");

		switch (ClassLibrary.getCurrent()) {
		case GNU_CLASSPATH:
			// executing createDerivedFoo
			expectRequest(ld2, "DerivedFoo")
				.expectDelegation(ld1)
					// ...linking (ld2, DerivedFoo)
					.expectRequest("Foo")
						.expectRequest("java.lang.Object")
						.expectDelegateToSystem()
					.expectDefinition()
				.expectDefinition()
			.expectLoaded();

			checkStringGetter(cls, "getDerivedFoo", "no exception");

			// subtype check (DerivedFoo subtypeof Foo)
			expectRequest(ld2, "Foo")
				.expectDelegation(ld1)
				.expectFound()
			.expectLoaded();

			checkStringGetter(cls, "getDerivedFooAsFoo", "no exception");
			break;
		case OPEN_JDK:
			// constructor of java.lang.Method checks descriptor of getDerivedFoo
			// this forces loading of Foo and String
			expectRequest(ld2, "java.lang.String")
			.expectDelegateToSystem();

			expectRequest(ld2, "Foo")
				.expectDelegation(ld1)
					.expectRequest("java.lang.Object")
					.expectDelegateToSystem()
				.expectDefinition()
			.expectLoaded();

			// executing createDerivedFoo
			expectRequest(ld2, "DerivedFoo")
				.expectDelegation(ld1)
				.expectDefinition()
			.expectLoaded();

			checkStringGetter(cls, "getDerivedFoo", "no exception");

			checkStringGetter(cls, "getDerivedFooAsFoo", "no exception");
			break;
		}

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