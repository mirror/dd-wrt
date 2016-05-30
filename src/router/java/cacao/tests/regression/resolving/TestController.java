/* regression/resolving/TestController.java - test harness

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

import java.util.*;
import java.lang.reflect.*;

// TestController is used to build a list of expected class loading events via ExpectationBuilder.
// During testing a TestLoader instance then emits the actual events via the report* methods.
public class TestController {
	//********************************************************************************************//
	//***** TEST SETUP

	// Expect that loader will be asked to load class with name classname.
	ExpectationBuilder expectRequest(ClassLoader loader, String classname) {
		expect(new Request(loader, classname));

		return new ExpectationBuilder(null, loader, classname);
	}

	// Expect that a given exception occurs
	public void expectException(Throwable exception) {
		expect(new ExceptionEvent(exception));
	}

	// Check if all expected events have occurred
	public void expectEnd() {
		if (_expectations.isEmpty()) {
			ok("got all expected reports");
		} else {
			fail("missing reports");

			for (Event e : _expectations) {
				fail("\t expected: " + e);
			}
		}
	}

	// Used to build the chain of expected class loading events for a test.
	//
	// Since class loading consists of recursive calls to loadClass the expectations also form a 
	// stack.
	// Every frame of this expectation stack contains a class loader and a class name.
	// The methods expectRequest and expectDelegation push a stack frame.
	// The methods expectLoaded, expectDefinition, expectDelegateToSystem and expectFound
	// pop the current stack frame.
	// The number of pushs and pops for one class loading must be equal, unless loading
	// is interrupted by an exception, signal this with TestController.expectException.
	public class ExpectationBuilder {
		// Expect that the current class loader will be asked to load class with name classname.
		// Corresponds to a recursive loadClass call caused by, for example, loading a classes
		// super class.
		public ExpectationBuilder expectRequest(String classname) {
			expect(new Request(this.loader, classname));

			return new ExpectationBuilder(this, this.loader, classname);
		}

		// Expect that the a given loader will be asked to load the current class.
		// Corresponds to an recursive loadClass call of another class loader.
		public ExpectationBuilder expectDelegation(ClassLoader delegate) {
			expect(new Delegation(loader, delegate, classname));
			expect(new Request(delegate, classname));

			return new ExpectationBuilder(this, delegate, classname);
		}

		// Expect that current class loader has finished loading the current class,
		// you only need this to balance out the pushs and pops when using expectDelegation.
		public ExpectationBuilder expectLoaded() {
			expect(new FinishedLoading(loader, classname));

			return parent;
		}

		// Expect that current class loader has defined the current class.
		public ExpectationBuilder expectDefinition() {
			expect(new Definition(loader, classname));

			return expectLoaded();
		}

		// Expect that current class loader has defined the current class.
		// And that the current class has a given class id.
		public ExpectationBuilder expectDefinitionWithClassId(String classId) {
			expect(new Definition(loader, classname + ":" + classId));
			expect(new FinishedLoading(loader, classname + ":" + classId));

			return parent;
		}

		// Expect that the current class loader has delegated loading the current class to the
		// system class loader.
		public ExpectationBuilder expectDelegateToSystem() {
			expect(new Delegation(loader, ClassLoader.getSystemClassLoader(), classname));

			return expectLoaded();
		}

		// Expect that the current class loader has found that the current class has already been
		// loaded.
		public ExpectationBuilder expectFound() {
			expect(new FoundLoaded(loader, classname));

			return expectLoaded();
		}

		// private parts

		private ExpectationBuilder(ExpectationBuilder parent, ClassLoader loader, String classname) {
			this.parent    = parent;
			this.loader    = loader;
			this.classname = classname;
		}

		private final ExpectationBuilder parent;
		private final ClassLoader        loader;
		private final String             classname;
	}

	//********************************************************************************************//
	//***** REPORTING CLASS LOADING EVENTS

	// report that loader has been asked to load class
	public void reportRequest(ClassLoader loader, String classname) {
		match(new Request(loader, classname));
	}

	// report that loader will delegate loading of class.
	public void reportDelegation(ClassLoader loader, ClassLoader delegate, String classname) {
		match(new Delegation(loader, delegate, classname));
	}

	// report that loader has defined class
	public void reportDefinition(ClassLoader loader, Class<?> cls) {
		match(new Definition(loader, className(cls)));
	}

	// report that loader has found class was already loaded
	public void reportFoundLoaded(ClassLoader loader, Class<?> cls) {
		match(new FoundLoaded(loader, className(cls)));
	}

	// report that loader has finished loading a class
	public void reportLoaded(ClassLoader loader, Class<?> cls) {
		match(new FinishedLoading(loader, className(cls)));
	}

	// report that a java.lang.ClassNotFoundException has occurred
	public void reportClassNotFound(ClassLoader loader, String classname, ClassNotFoundException e) {
		if (!match(new NotFound(loader, classname)))
			e.printStackTrace(System.out);
	}

	// report that an exception has occurred
	public void reportException(Throwable ex) {
		if (!match(new ExceptionEvent(ex)))
			ex.printStackTrace(System.out);
	}

	//********************************************************************************************//
	//***** MISC. TEST HELPERS

	// set if testing should log and check class ids
	public void setReportClassIDs(boolean rep) {
		_report_class_ids = rep;
	}

	// call static no argument string getter function on class and check if it returns
	// a given string
	public void checkStringGetter(Class<?> cls, String methodname, String expected) {
		String id = invokeStringGetter(cls, methodname);
		if (id == null)
			fail("could not get return value of " + methodname + "()");
		else if (id.equals(expected))
			ok("returned string matches: " + id);
		else
			fail("wrong string returned: " + id + ", expected: " + expected);
	}

	// check that calling a static no argument string getter function on class fails
	public void checkStringGetterMustFail(Class<?> cls, String methodname) {
		String id = invokeStringGetter(cls, methodname);
		if (id == null)
			ok("method invocation failed as expected: " + methodname + "()");
		else 
			fail("method invocation did not fail as expected: " + methodname + "()");
	}

	// check that a class has a given class id
	public void checkClassId(Class<?> cls, String expected) {
		String id = getClassId(cls);
		if (id == null)
			fail("could not get class id");
		else if (id.equals(expected))
			ok("class id matches: " + id);
		else
			fail("wrong class id: " + id + ", expected: " + expected);
	}

	// load a class with loader
	public Class<?> loadClass(ClassLoader loader, String classname) {
		try {
			return loader.loadClass(classname);
		} catch (ClassNotFoundException e) {
			reportClassNotFound(loader, classname, e);
			return null;
		}
	}

	// finish test
	public void exit() {
		expectEnd();
		System.exit(_failed ? 1 : 0);
	}

	// ***** private parts

	private abstract class Event {
		public abstract boolean matches(Event e);

		protected final boolean equal(Object a, Object b) {
			if (a == null)
				return b == null;

			if (b == null)
			return a == null;

			return a.equals(b);
		}
	}

	private abstract class LoadingEvent extends Event {
		final ClassLoader loader;
		final String      classname;

		LoadingEvent(ClassLoader loader, String classname) {
			this.loader    = loader;
			this.classname = classname;
		}

		public boolean matches(Event e) {
			if (e.getClass() != getClass())
				return false;

			LoadingEvent le = (LoadingEvent) e;

			return equal(loader,    le.loader)
				&& equal(classname, le.classname);
		}
	}

	private final class Request extends LoadingEvent {
		Request(ClassLoader loader, String classname) {
			super(loader, classname);
		}

		public String toString() {
			return String.format("requested: %s.loadClass(%s)", loaderName(loader), classname);
		}
	}	
	private final class Definition extends LoadingEvent {
		Definition(ClassLoader loader, String classname) {
			super(loader, classname);
		}

		public String toString() {
			return String.format("defined: %s.defineClass(%s)", loaderName(loader), classname);
		}
	}
	private final class FoundLoaded extends LoadingEvent {
		FoundLoaded(ClassLoader loader, String classname) {
			super(loader, classname);
		}

		public String toString() {
			return String.format("found: %s already loaded %s", loaderName(loader), classname);
		}
	}
	private final class FinishedLoading extends LoadingEvent {
		FinishedLoading(ClassLoader loader, String classname) {
			super(loader, classname);
		}

		public String toString() {
			return String.format("loaded: %s finished loading %s", loaderName(loader), classname);
		}
	}
	private final class NotFound extends LoadingEvent {
		NotFound(ClassLoader loader, String classname) {
			super(loader, classname);
		}

		public String toString() {
			return String.format("class not found: %s could not find %s", loaderName(loader), classname);
		}
	}

	private final class Delegation extends LoadingEvent {
		final ClassLoader delegate;

		public Delegation(ClassLoader loader, ClassLoader delegate, String classname) {
			super(loader, classname);
			
			this.delegate = delegate;
		}

		public boolean matches(Event e) {
			if (!super.matches(e))
				return false;

			Delegation de = (Delegation) e;

			return equal(delegate,  de.delegate);
		}

		public String toString() {
			return String.format("delegated: %s.loadClass(%s) -> %s.loadClass(%s)", loaderName(loader), classname, loaderName(delegate), classname);
		}
	}

	private final class ExceptionEvent extends Event {
		final Throwable exception;

		public ExceptionEvent(Throwable t) {
			exception = t;
		}

		public boolean matches(Event e) {
			if (e.getClass() != ExceptionEvent.class)
				return false;

			ExceptionEvent ee = (ExceptionEvent) e;

			return equal(exception.toString(), ee.exception.toString());
		}

		public String toString() {
			return String.format("exception: %s", exception);
		}
	}

	private synchronized boolean match(Event event) {
		if (_expectations.isEmpty()) {
			fail("unexpected", event);
			return false;
		} else {
			Event expected = _expectations.peek();

			if (expected.matches(event)) {
				_expectations.remove();
				ok(event);
				return true;
			} else {
				fail("unmatched", event, expected);
				return false;
			}
		}
	}

	private void expect(Event e) {
		_expectations.add(e);
	}

	private void fail(Object message) {
		log("FAIL: " + message);
		_failed = true;
	}
	private void fail(String message, Object o) {
		fail(message + ": " + o);
	}
	private void fail(String message, Object actual, Object expected) {
		fail(message + ": " + actual + " (expected " + expected + ")");
	}

	private void ok(Object message) {
		log("ok: " + message);
	}
	private void ok(String tag, String ld1, String ld2, String cls) {
		ok(tag + " " + ld1 + " " + ld2 + " class=" + cls);
	}

	private void log(String str) {
		System.out.println(str);
	}

	private String loaderName(ClassLoader loader) {
		if (loader == ClassLoader.getSystemClassLoader())
			return "SystemClassLoader";

		return (loader == null) ? "null" : loader.toString();
	}

	private String getClassId(Class<?> cls) {
		return invokeStringGetter(cls, "id");
	}

	private String className(Class<?> cls) {
		if (_report_class_ids) {
			String id = getClassId(cls);
			if (id != null)
				return cls.getName() + ":" + id;
		}

		return cls.getName();
	}

	private String exceptionName(Throwable t) {
		return t.toString();
	}

	private String invokeStringGetter(Class<?> cls, String methodname) {
		try {
			Method mid = cls.getMethod(methodname);

			String id = (String) mid.invoke(null);

			return id;
		}
		catch (NoSuchMethodException e) {
			return null;
		}
		catch (InvocationTargetException e) {
			reportException(e.getCause());
			return null;
		}
		catch (Throwable t) {
			reportException(t);
			return null;
		}
	}

	final Queue<Event> _expectations     = new LinkedList<Event>();
	boolean            _report_class_ids = false;
	boolean            _failed           = false;
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