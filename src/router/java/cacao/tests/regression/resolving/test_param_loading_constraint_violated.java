/* regression/resolving/test_param_loading_constraint_violated.java

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

public class test_param_loading_constraint_violated extends TestController {

	public static void main(String[] args) {
		new test_param_loading_constraint_violated();
	}

	test_param_loading_constraint_violated() {
		// ***** setup

		TestLoader ld1 = new TestLoader("ld1", this);
		TestLoader ld2 = new TestLoader("ld2", this);

		ld1.addClassfile("BarUseFoo", "classes1/BarUseFoo.class");
		ld1.addClassfile("Foo",       "classes1/Foo.class");
		ld1.addParentDelegation("java.lang.Object");
		ld1.addParentDelegation("java.lang.String");

		ld2.addClassfile("BarPassFoo", "classes2/BarPassFoo.class");
		ld2.addClassfile("Foo",        "classes2/Foo.class");
		ld2.addDelegation("BarUseFoo", ld1);
		ld2.addParentDelegation("java.lang.Object");
		ld2.addParentDelegation("java.lang.String");

		// OpenJDKs reflection API causes us to load DerivedFoo
		if (ClassLibrary.getCurrent() == ClassLibrary.OPEN_JDK)
			ld2.addClassfile("DerivedFoo", "classes2/DerivedFoo.class");

		// loading BarPassFoo
		expectRequest(ld2, "BarPassFoo")
			// linking BarPassFoo
			.expectRequest("java.lang.Object")
			.expectDelegateToSystem()
		.expectDefinition();

		Class<?> cls = loadClass(ld2, "BarPassFoo");

		switch (ClassLibrary.getCurrent()) {
		case GNU_CLASSPATH:
			// executing BarPassFoo.passit: new Foo
			expectRequest(ld2, "Foo")
			.expectDefinition();

			// executing BarPassFoo.passit: new BarUseFoo
			expectRequest(ld2, "BarUseFoo")
				.expectDelegation(ld1)
					// ...linking BarUseFoo
					.expectRequest("java.lang.Object")
					.expectDelegateToSystem()
				.expectDefinition()
			.expectLoaded();

			// resolving Foo.virtualId() from BarUseFoo
			expectRequest(ld1, "Foo");
			break;
		case OPEN_JDK:
			// constructor of java.lang.Method checks descriptor of passit
			// this forces loading of Foo and String
			expectRequest(ld2, "java.lang.String")
			.expectDelegateToSystem();

			// executing BarPassFoo.passit: new Foo
			expectRequest(ld2, "Foo")
			.expectDefinition();

			expectRequest(ld2, "DerivedFoo")
			.expectDefinition();

			// executing BarPassFoo.passit: new BarUseFoo
			expectRequest(ld2, "BarUseFoo")
				.expectDelegation(ld1)
					// ...linking BarUseFoo
					.expectRequest("java.lang.Object")
					.expectDelegateToSystem()
				.expectDefinition()
			.expectLoaded();

			// resolving Foo.virtualId() from BarUseFoo
			expectRequest(ld1, "Foo");
			break;
		}

		// the loading constraing (ld1,ld2,Foo) is violated
		expectException(new LinkageError("Foo: loading constraint violated: "));

		checkStringGetterMustFail(cls, "passit");

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