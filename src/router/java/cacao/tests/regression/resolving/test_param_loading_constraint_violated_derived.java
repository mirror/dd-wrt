/* regression/resolving/test_param_loading_constraint_violated_derived.java

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

public class test_param_loading_constraint_violated_derived extends TestController {

	public static void main(String[] args) {
		new test_param_loading_constraint_violated_derived();
	}

	test_param_loading_constraint_violated_derived() {
		// ***** setup

		TestLoader ld1 = new TestLoader("ld1", this);
		TestLoader ld2 = new TestLoader("ld2", this);
		TestLoader ld3 = new TestLoader("ld3", this);

		ld1.addClassfile("BarUseFoo", "classes1/BarUseFoo.class");
		ld1.addClassfile("Foo",       "classes1/Foo.class");
		ld1.addParentDelegation("java.lang.Object");
		ld1.addParentDelegation("java.lang.String");

		ld2.addClassfile("BarPassFoo", "classes2/BarPassFoo.class");
		ld2.addDelegation("BarUseFoo",  ld1);
		ld2.addDelegation("Foo",        ld3);
		ld2.addDelegation("DerivedFoo", ld3);
		ld2.addParentDelegation("java.lang.Object");
		ld2.addParentDelegation("java.lang.String");

		ld3.addClassfile("Foo",        "classes3/Foo.class");
		ld3.addClassfile("DerivedFoo", "classes3/DerivedFoo.class");
		ld3.addParentDelegation("java.lang.Object");
		ld3.addParentDelegation("java.lang.String");

		// loading & linking BarPassFoo
		expectRequest(ld2, "BarPassFoo")
			.expectRequest("java.lang.Object")
			.expectDelegateToSystem()
		.expectDefinition();

		Class<?> cls = loadClass(ld2, "BarPassFoo");

		switch (ClassLibrary.getCurrent()) {
		case GNU_CLASSPATH:
			// executing BarPassFoo.passDerivedFoo: new DerivedFoo
			expectRequest(ld2, "DerivedFoo")
				.expectDelegation(ld3)
					// linking (ld3, DerivedFoo)
					.expectRequest("Foo")
						.expectRequest("java.lang.Object")
						.expectDelegateToSystem()
					.expectDefinition()
				.expectDefinition()
			.expectLoaded();

			// executing BarPassFoo.passit: new BarUseFoo
			expectRequest(ld2, "BarUseFoo")
				.expectDelegation(ld1)
					// linking BarUseFoo
					.expectRequest("java.lang.Object")
					.expectDelegateToSystem()
				.expectDefinition()
			.expectLoaded();

			// resolving BarUseFoo.useFoo
			// the deferred subtype check ((ld2, DerivedFoo) subtypeof (ld2, Foo)) is done
			expectRequest(ld2, "Foo")
				.expectDelegation(ld3)
				.expectFound()
			.expectLoaded();
			break;
		case OPEN_JDK:
			// constructor of java.lang.Method checks descriptor of passDerivedFoo
			// this forces loading of Foo and String
			expectRequest(ld2, "java.lang.String")
			.expectDelegateToSystem();

			expectRequest(ld2, "Foo")
				.expectDelegation(ld3)
					.expectRequest("java.lang.Object")
					.expectDelegateToSystem()
				.expectDefinition()
			.expectLoaded();

			expectRequest(ld2, "DerivedFoo")
				.expectDelegation(ld3)
				.expectDefinition()
			.expectLoaded();

			// executing BarPassFoo.passit: new BarUseFoo
			expectRequest(ld2, "BarUseFoo")
				.expectDelegation(ld1)
					// linking BarUseFoo
					.expectRequest("java.lang.Object")
					.expectDelegateToSystem()
				.expectDefinition()
			.expectLoaded();
			break;
		}

		// resolving Foo.virtualId() from BarUseFoo
		expectRequest(ld1, "Foo");

		// the loading constraint (ld1,ld2,Foo) is violated
		expectException(new LinkageError("Foo: loading constraint violated: "));

		checkStringGetterMustFail(cls, "passDerivedFoo");

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